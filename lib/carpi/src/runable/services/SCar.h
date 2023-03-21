#ifndef SCAR_H
#define SCAR_H

#include "platform/CircularBuffer.h"
#include "IService.h"
#include "runable/programs/PCockpitIndicator.h"
#include "runable/programs/PBrakeLight.h"
#include "runable/services/SLed.h"
#include "communication/componentIds.h"
#include "communication/CANService.h"
#include "components/interface/IButton.h"
#include "components/interface/ILed.h"
#include "components/interface/IBuzzer.h"
#include "components/interface/IMotorController.h"
#include "components/interface/IHvEnabled.h"
#include "PowerModes.h"

#define STARTUP_WAIT 1                   // s wait before system gets started
#define ERROR_REGISTER_SIZE 64           // errors, max: 255
#define STARTUP_ANIMATION_WAIT_AFTER 0.3 // s wait after animation
#define BRAKE_START_THRESHHOLD 0.60      // %

#define MAX_BUTTON_STATE_AGE 0.5 // s

#define LAUNCH_CONTROL_RELEASE_VALUE 0.95 // % where the gas is activated again

#define HV_ENABLED_BEEP_TIME 2.2 // s (has to be at least 0.5)

#define BEEP_MULTI_BEEP_TIME 0.1    // s
#define BEEP_MULTI_SILENT_TIME 0.08 // s

#define LED_RESEND_INTERVAL_IN_RUN 0.33 // s

enum gas_curve_t : uint8_t
{
    GAS_CURVE_LINEAR = 0x0,
    GAS_CURVE_X_POW_2,
    GAS_CURVE_X_POW_3,
    GAS_CURVE_X_POW_4
};

enum car_state_t : uint8_t
{
    CAR_OFF = 0x0,
    BOOT = 0x1,
    ALMOST_READY_TO_DRIVE = 0x2,
    READY_TO_DRIVE = 0x3,
    CAR_ERROR = 0x4,
    CALIBRATION_NEEDED = 0x5,
    LAUNCH_CONTROL = 0x6
};

enum error_type_t : uint8_t
{
    ERROR_NO = 0x0,
    ERROR_UNDEFINED = 0x1,
    ERROR_ISSUE = 0x2,
    ERROR_SYSTEM = 0x3,  // buffer-overflow...
    ERROR_CRITICAL = 0x4 // Pedal-Error -> not ready to drive anymore
};

class Error
{
public:
    Error() {}
    Error(id_component_t ComponentId, uint8_t Code, error_type_t Type)
        : componentId(ComponentId), code(Code), type(Type) {}

    id_component_t componentId = 0;
    uint8_t code = 0;
    error_type_t type = ERROR_NO;
};

class SCar : public IService
{
public:
    SCar(CANService &canService, SLed &ledService,
         IButton *buttonReset, IButton *buttonStart,
         IPedal *gasPedal, IPedal *brakePedal,
         IBuzzer *buzzer,
         IMotorController *motorController,
         IHvEnabled *hvEnabled,
         IHvEnabled *tsms,
         PBrakeLight &brakeLightService)
        : _canService(canService), _ledService(ledService), _brakeLightService(brakeLightService)
    {
        _button.reset = buttonReset;
        _button.start = buttonStart;

        _pedal.gas = gasPedal;
        _pedal.brake = brakePedal;

        _buzzer = buzzer;

        _motorController = motorController;

        _hvEnabled = hvEnabled;
        _tsms = tsms;
    }

    virtual void run()
    {
        _checkHvEnabled();

        _checkInput();

        if (_button.reset->getStatus() > 0)
        {
            addError(Error(_calculateComponentId((IComponent *)_button.reset), _button.reset->getStatus(), ERROR_SYSTEM));
        }

        if (_button.start->getStatus() > 0)
        {
            addError(Error(_calculateComponentId((IComponent *)_button.start), _button.start->getStatus(), ERROR_SYSTEM));
        }

        if (!(_errorRegister.empty()))
        {
            processErrors();
        }
    }

    void addError(Error error)
    {
        _errorRegister.push(error);
    }

    void processErrors()
    {
        while (!_errorRegister.empty())
        {
            Error error;
            _errorRegister.pop(error);

#ifdef REPORT_ERRORS
            pcSerial.printf("[SCar]@processErrors: Got Error at 0x%x with error code 0x%x and error type 0x%x !\n", error.componentId, error.code, error.type);
#endif

            if (error.type >= ERROR_CRITICAL)
            {
                // Critical Error
                // Red -> Fast Blinking
                // Green -> Off
                // Car -> Error -> No driving anymore, Error must be solved

                _ledService.resetLeds();

                _ledService._led.red->setState(LED_ON);
                _ledService._led.red->setBrightness(1);
                _ledService._led.red->setBlinking(BLINKING_FAST);

                _ledService._led.green->setState(LED_OFF);
                _state = CAR_ERROR;

                _motorController->setRUN(MOTOR_CONTROLLER_RUN_DISABLE);
                _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);

                _ledService.run();
            }
        }
    }

    car_state_t getState()
    {
        return _state;
    }

    void calibrationNeeded(bool errorByCommunication = false)
    {
        if (_state == READY_TO_DRIVE)
        {
            _motorController->setRUN(MOTOR_CONTROLLER_RUN_DISABLE);
            _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);
        }

        _state = CALIBRATION_NEEDED;
        _ledService.resetLeds();
        _ledService._led.yellow->setState(LED_ON);
        _ledService._led.yellow->setBlinking(BLINKING_NORMAL);

        if (errorByCommunication)
        {
            _ledService._led.red->setState(LED_ON);
            _ledService._led.red->setBlinking(BLINKING_FAST);
        }

        _ledService.run();
    }

    void startUp()
    {
        wait(STARTUP_WAIT);
        
        _ledService.startupAnimation();

        wait(STARTUP_ANIMATION_WAIT_AFTER);

        // Turn on red LED while HV-Circuite is off
        _ledService.resetLeds();
        _ledService._led.red->setState(LED_ON);
        _ledService.run();
    }

    /*
    void startUp()
    {
        wait(STARTUP_WAIT);

        _brakeLightService.run();

        for (uint8_t i = 0; i < STARTUP_ANIMATION_PLAYBACKS; i++)
        {
            _startupAnimation();

            _brakeLightService.run();
        }

        wait(STARTUP_ANIMATION_WAIT_AFTER);

        // Turn on red LED while HV-Circuite is off
        _resetLeds();
        _led.red->setState(LED_ON);
        _sendLedsOverCan();

        // [QF]
        Timer _ledResendWaitTimer;
        _ledResendWaitTimer.reset();
        _ledResendWaitTimer.start();
        while ((!(_hvEnabled->read()) || !(_tsms->read())) && !(_button.start->getState() == LONG_CLICKED && _button.start->getStateAge() < MAX_BUTTON_STATE_AGE))
        {
            _canService.processInbound();
            processErrors();
            _brakeLightService.run();

            if (_ledResendWaitTimer.read() >= 0.3)
            {
                _ledResendWaitTimer.reset();
                _ledResendWaitTimer.start();
                _sendLedsOverCan();
            }

            wait(0.001);
        }

        _resetLeds();
        _sendLedsOverCan();

        wait(0.1);

        _canService.processInbound();
        _checkHvEnabled();
        _state = ALMOST_READY_TO_DRIVE;
        processErrors();
        if (_state != ALMOST_READY_TO_DRIVE)
        {
            // If an Error occured, stop continuing and glow Red
            // Red   -> Blinking Fast
            // Green -> Off
            _led.red->setState(LED_ON);
            _led.red->setBlinking(BLINKING_FAST);
            _led.green->setState(LED_OFF);
            _sendLedsOverCan();
            while (1)
            {

                _brakeLightService.run();
                _sendLedsOverCan();
                wait(0.3);
            }
        }

        _led.green->setState(LED_ON);
        _led.green->setBlinking(BLINKING_FAST);
        _sendLedsOverCan();
    }
    */

    gas_curve_t getGasCurve()
    {
        return _gasCurve;
    }

#ifdef ENABLE_POWER_MENU
    uint8_t getCurrentModeId()
    {
        return _currentModeId;
    }

    float getMaxPower()
    {
        return maxAllowedPower;
    }

    float getMaxAmpere()
    {
        return maxAllowedAmpere;
    }

    float getMaxTorque()
    {
        return maxAllowedTorque;
    }

#ifdef EXPERIMENTELL_ASR_ACTIVE
    bool getAsrOn()
    {
        return asrOn;
    }
#endif
#endif

private:
    CANService &_canService;
    SLed &_ledService;

    CircularBuffer<Error, ERROR_REGISTER_SIZE, uint8_t> _errorRegister;

    car_state_t _state = CAR_OFF;

    gas_curve_t _gasCurve = GAS_CURVE_X_POW_2;

#ifdef ENABLE_POWER_MENU
    uint8_t _currentModeId = 0;
    // Settings for Power Modes (driving, recu is static)
    float maxAllowedAmpere = MODE_0_ACCU_MAX_ALLOWED_CURRENT;
    float maxAllowedPower = MODE_0_ACCU_MAX_ALLOWED_POWER;
    float maxAllowedTorque = MODE_0_MOTOR_MAX_ALLOWED_TORQUE;
#ifdef EXPERIMENTELL_ASR_ACTIVE
    bool asrOn = false;
#endif
#endif

    struct _button
    {
        IButton *reset;
        IButton *start;
    } _button;

    struct _pedal
    {
        IPedal *gas;
        IPedal *brake;
    } _pedal;

    IBuzzer *_buzzer;

    IMotorController *_motorController;

    IHvEnabled *_hvEnabled;
    IHvEnabled *_tsms;

    PBrakeLight &_brakeLightService;

    id_component_t _calculateComponentId(IComponent *component)
    {
        id_component_t id = component->getComponentId();
        return id;
    }

    void _sendLedsOverCan()
    {
       _ledService.run();
    }

    void _sendPedalsOverCan()
    {
        // Pedals
        _canService.sendMessage((ICommunication *)_pedal.gas, DEVICE_PEDAL);
        _canService.sendMessage((ICommunication *)_pedal.brake, DEVICE_PEDAL);
    }

    void _sendComponentsOverCan()
    {
        _ledService.run();
        _sendPedalsOverCan();
    }

    void _pedalError(IPedal *sensorId)
    {
        addError(Error(sensorId->getComponentId(), sensorId->getStatus(), ERROR_ISSUE));
        calibrationNeeded();
    }

    void _checkHvEnabled()
    {
        // [QF]
        static bool hvEnabledErrorAdded = false;
        static bool tsmsErrorAdded = false;

        if (!(_hvEnabled->read()) || !(_tsms->read()))
        {
            if (_state == READY_TO_DRIVE)
            {
                _state = ALMOST_READY_TO_DRIVE;
                _motorController->setRUN(MOTOR_CONTROLLER_RUN_DISABLE);
                _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);
                _ledService._led.green->setBlinking(BLINKING_FAST);
                _ledService.run();
            }

            if (!hvEnabledErrorAdded)
            {
                addError(Error(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_60V_OK), 0x1, ERROR_ISSUE));
                hvEnabledErrorAdded = true;
            }
            if (!tsmsErrorAdded)
            {
                addError(Error(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_TSMS), 0x1, ERROR_ISSUE));
                tsmsErrorAdded = true;
            }
        }
        else
        {
            hvEnabledErrorAdded = false;
            tsmsErrorAdded = false;
        }
    }

    void _calibratePedals()
    {
        // Start bootup/calibration
        // Yellow -> On
        // Green  -> Normal Blinking
        _state = BOOT;
        _ledService.resetLeds();
        _ledService._led.yellow->setState(LED_ON);
        _ledService._led.green->setState(LED_ON);

        _ledService._led.yellow->setBlinking(BLINKING_OFF);
        _ledService._led.yellow->setBrightness(0.76);
        _ledService._led.green->setBlinking(BLINKING_NORMAL);

        _pedal.gas->setCalibrationStatus(CURRENTLY_CALIBRATING);
        _pedal.brake->setCalibrationStatus(CURRENTLY_CALIBRATING);

        _sendComponentsOverCan();

        // Calibrate pedal until pressed Start once for long time
        Timer ledSendTimer;
        ledSendTimer.reset();
        ledSendTimer.start();
        while (!(_button.start->getState() == LONG_CLICKED && _button.start->getStateAge() < MAX_BUTTON_STATE_AGE))
        {
            _canService.processInbound();

            if (ledSendTimer.read() >= 0.3)
            {
                ledSendTimer.reset();
                ledSendTimer.start();
                _ledService.run();
            }

            wait(0.001);
        }

        // Stop calibration
        _pedal.gas->setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);
        _pedal.brake->setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);
        _sendPedalsOverCan();

        wait(0.1);
        _canService.processInbound();
        _ledService._led.yellow->setState(LED_OFF);
        _ledService.run();

        // Wait till the Button got released again
        // Yellow -> Off
        while (_button.start->getState() != NOT_PRESSED)
        {
            _canService.processInbound();

            if (ledSendTimer.read() >= 0.3)
            {
                ledSendTimer.reset();
                ledSendTimer.start();
                _ledService.run();
            }

            wait(0.001);
        }

        wait(0.1);

        // Check for Errors at Calibration
        _canService.processInbound();
        if (_pedal.gas->getStatus() > 0)
        {
            _pedalError(_pedal.gas);
        }

        if (_pedal.brake->getStatus() > 0)
        {
            _pedalError(_pedal.brake);
        }

        processErrors();

        // If all OK, go into Almost Ready to drive
        if (_state == BOOT)
        {
            _state = ALMOST_READY_TO_DRIVE;
        }
        else
        {
            _ledService.resetLeds();
            _ledService._led.red->setState(LED_ON);
            _ledService.run();
            wait(0.5);
            calibrationNeeded();
            return;
        }

        // If no Error, start blinking fast to show ready, but need to start car
        // Green -> Fast Blinking
        _ledService.resetLeds();
        _ledService._led.green->setState(LED_ON);
        _ledService._led.green->setBlinking(BLINKING_FAST);
        _ledService.run();

        wait(0.1);
    }

#ifdef ENABLE_POWER_MENU
    bool resetWasPressedBefore = false;
    bool resetWasNotPressedBefore = false;
#endif
    void _checkInput()
    {
        if (_state == CAR_OFF)
        {
            // [QF]
            Timer _ledResendWaitTimer;
            _ledResendWaitTimer.reset();
            _ledResendWaitTimer.start();
            
            if ((!(_hvEnabled->read()) || !(_tsms->read())) && !(_button.start->getState() == LONG_CLICKED && _button.start->getStateAge() < MAX_BUTTON_STATE_AGE)){
                processErrors();
            }else{
                _canService.processInbound();
                _checkHvEnabled();
                _state = ALMOST_READY_TO_DRIVE;
                processErrors();
                if (_state != ALMOST_READY_TO_DRIVE)
                {
                    // If an Error occured, stop continuing and glow Red
                    // Red   -> Blinking Fast
                    // Green -> Off
                    _ledService._led.red->setState(LED_ON);
                    _ledService._led.red->setBlinking(BLINKING_FAST);
                    _ledService._led.green->setState(LED_OFF);
                    _ledService.run();
                    while (1)
                    {

                        _brakeLightService.run();
                        _ledService.run();
                        wait(0.3);
                    }
                }

                _ledService._led.green->setState(LED_ON);
                _ledService._led.green->setBlinking(BLINKING_FAST);
                _ledService.run();
            }
            
        }
        // [QF]
        if (_state == ALMOST_READY_TO_DRIVE)
        {

            if ((_button.start->getState() == LONG_CLICKED && _button.start->getStateAge() < MAX_BUTTON_STATE_AGE) && (_pedal.brake->getValue() >= BRAKE_START_THRESHHOLD) && (_hvEnabled->read()) && (_tsms->read()))
            {
                // Optimize later!!
                // [il]
                // Set RFE enable
                _motorController->setRFE(MOTOR_CONTROLLER_RFE_ENABLE);

                // [QF]
                // Start beeping to signalize car is started
                _buzzer->setBeep(BUZZER_MONO_TONE);
                _buzzer->setState(BUZZER_ON);

                // Wait the set Time until enabling RUN
                Timer waitTimer;
                waitTimer.reset();
                waitTimer.start();

                Timer ledSendTimer;
                ledSendTimer.reset();
                ledSendTimer.start();

                do
                {
                    _canService.processInbound();
                    _checkHvEnabled();
                    _brakeLightService.run();
                    processErrors();

                    if (ledSendTimer.read() >= 0.3)
                    {
                        ledSendTimer.reset();
                        ledSendTimer.start();
                        _ledService.run();
                    }

                    if (_state != ALMOST_READY_TO_DRIVE)
                    {
                        _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);
                        // If an Error occured, stop continuing and glow Red
                        // Red   -> Blinking Normal
                        // Green -> Off
                        _ledService.resetLeds();
                        _ledService._led.red->setState(LED_ON);
                        _ledService._led.red->setBlinking(BLINKING_NORMAL);
                        _ledService.run();

                        // Stop beeping!
                        _buzzer->setState(BUZZER_OFF);

                        _brakeLightService.run();
                        wait(1);

                        calibrationNeeded();
                        return;
                    }
                } while (waitTimer.read() < (float)HV_ENABLED_BEEP_TIME);

                // Set RUN enable if no Error
                _canService.processInbound();
                _checkHvEnabled();
                processErrors();

                // Stop beeping!
                _buzzer->setState(BUZZER_OFF);

                if (_state == ALMOST_READY_TO_DRIVE)
                {
                    _motorController->setRUN(MOTOR_CONTROLLER_RUN_ENABLE);
                }
                else
                {
                    _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);
                    // If an Error occured, stop continuing and glow Red
                    // Red   -> Blinking Normal
                    // Green -> Off
                    _ledService.resetLeds();
                    _ledService._led.red->setState(LED_ON);
                    _ledService._led.red->setBlinking(BLINKING_NORMAL);
                    _ledService.run();

                    _brakeLightService.run();
                    wait(1);

                    calibrationNeeded();
                    return;
                }

                // Set car ready to drive (-> pressing the gas-pedal will move the car -> fun)
                _state = READY_TO_DRIVE;

                // Stop blinking greed to show car is primed
                _ledService.resetLeds();
                _ledService._led.green->setState(LED_ON);
                _ledService.run();
                wait(0.01);
            }

            if (_button.reset->getState() == LONG_CLICKED && _button.reset->getStateAge() < MAX_BUTTON_STATE_AGE)
            {
#ifdef ENABLE_POWER_MENU
                resetWasPressedBefore = false;
#endif
                _calibratePedals();
            }

#ifdef ENABLE_POWER_MENU
            if (_button.reset->getState() == PRESSED && _button.reset->getStateAge() < MAX_BUTTON_STATE_AGE)
            {
                resetWasPressedBefore = true;
            }

            if (_button.reset->getState() == NOT_PRESSED && _button.reset->getStateAge() < MAX_BUTTON_STATE_AGE)
            {
                if (!resetWasNotPressedBefore)
                {
                    resetWasPressedBefore = false;
                    resetWasNotPressedBefore = true;
                }
                else
                {
                    if (resetWasPressedBefore)
                    {
                        resetWasPressedBefore = false;

                        if (resetWasNotPressedBefore)
                        {
                            resetWasNotPressedBefore = false;

                            _sycleModes();
                        }
                    }
                    else
                    {
                        resetWasNotPressedBefore = true;
                    }
                }
            }
#endif

            return;
        }
        else
        {
#ifdef ENABLE_POWER_MENU
            resetWasNotPressedBefore = false;
            resetWasPressedBefore = false;
#endif
        }

        if (_state == CALIBRATION_NEEDED)
        {
            // Start Calibration after Calibration failure, clearing the error at beginning on pedals
            if (_button.reset->getState() == LONG_CLICKED && _button.reset->getStateAge() < MAX_BUTTON_STATE_AGE)
            {
                _calibratePedals();
            }
        }

        if (_state == READY_TO_DRIVE)
        {
            if (_button.reset->getState() == PRESSED)
            {
                // Disable RFE and RUN
                _motorController->setRUN(MOTOR_CONTROLLER_RUN_DISABLE);
                _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);

                _state = ALMOST_READY_TO_DRIVE;

                _ledService._led.green->setBlinking(BLINKING_FAST);
                _ledService.run();

                return;
            }

            if ((_button.start->getState() == PRESSED && _button.start->getStateAge() < MAX_BUTTON_STATE_AGE) && (_pedal.brake->getValue() >= BRAKE_START_THRESHHOLD))
            {
                _beepTimes(10, 0.02, 0.02);
                _state = LAUNCH_CONTROL;
            }

            return;
        }

        if (_state == LAUNCH_CONTROL)
        {
            if (_pedal.gas->getValue() >= LAUNCH_CONTROL_RELEASE_VALUE)
            {
                _state = READY_TO_DRIVE;
            }

            if (_button.reset->getState() == PRESSED)
            {
                // Disable RFE and RUN
                _motorController->setRUN(MOTOR_CONTROLLER_RUN_DISABLE);
                _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);

                _state = ALMOST_READY_TO_DRIVE;

                _ledService._led.green->setBlinking(BLINKING_FAST);
                _ledService.run();

                return;
            }

            return;
        }
    }

    void _beepTimes(uint8_t times, float beepOnTime = BEEP_MULTI_BEEP_TIME, float beepOffTime = BEEP_MULTI_SILENT_TIME)
    {
        _buzzer->setBeep(BUZZER_MONO_TONE);
        for (uint8_t i = 0; i < times; i++)
        {
            _buzzer->setState(BUZZER_ON);
            wait(beepOnTime);
            _buzzer->setState(BUZZER_OFF);
            wait(beepOffTime);
        }
    }

#ifdef ENABLE_POWER_MENU
    void _sycleModes()
    {
        _currentModeId++;

#ifdef EXPERIMENTELL_ASR_ACTIVE
        if (_currentModeId > 4)
            _currentModeId = 0;
#else
        if (_currentModeId > 2)
            _currentModeId = 0;
#endif

        _beepTimes(_currentModeId + 1);

        switch (_currentModeId)
        {
        case 0:
            maxAllowedPower = MODE_0_ACCU_MAX_ALLOWED_POWER;
            maxAllowedAmpere = MODE_0_ACCU_MAX_ALLOWED_CURRENT;
            maxAllowedTorque = MODE_0_MOTOR_MAX_ALLOWED_TORQUE;

#ifdef EXPERIMENTELL_ASR_ACTIVE
            asrOn = false;
#endif
            break;

        case 1:
            maxAllowedPower = MODE_1_ACCU_MAX_ALLOWED_POWER;
            maxAllowedAmpere = MODE_1_ACCU_MAX_ALLOWED_CURRENT;
            maxAllowedTorque = MODE_1_MOTOR_MAX_ALLOWED_TORQUE;

#ifdef EXPERIMENTELL_ASR_ACTIVE
            asrOn = false;
#endif
            break;

        case 2:
            maxAllowedPower = MODE_2_ACCU_MAX_ALLOWED_POWER;
            maxAllowedAmpere = MODE_2_ACCU_MAX_ALLOWED_CURRENT;
            maxAllowedTorque = MODE_2_MOTOR_MAX_ALLOWED_TORQUE;

#ifdef EXPERIMENTELL_ASR_ACTIVE
            asrOn = false;
#endif
            break;

#ifdef EXPERIMENTELL_ASR_ACTIVE

        case 3:
            maxAllowedPower = MODE_3_ACCU_MAX_ALLOWED_POWER;
            maxAllowedAmpere = MODE_3_ACCU_MAX_ALLOWED_CURRENT;
            maxAllowedTorque = MODE_3_MOTOR_MAX_ALLOWED_TORQUE;

            asrOn = true;
            break;

        case 4:
            maxAllowedPower = MODE_4_ACCU_MAX_ALLOWED_POWER;
            maxAllowedAmpere = MODE_4_ACCU_MAX_ALLOWED_CURRENT;
            maxAllowedTorque = MODE_4_MOTOR_MAX_ALLOWED_TORQUE;

            asrOn = true;
            break;

#endif

        default:
            // What??
            break;
        }
    }
#endif

    /*
        Modes:
            0: Normal mode. 60kW, 200A max, 230 Nm
            1: Baby mode. 40kW, 120A max, 120 Nm
            2: High Power mode. 80kW, 250A max, 230 Nm
            3: ASR Easy Test. 60kW, 200A max, 180 Nm, ASR On
            4: ASR High Power Test. 80kW, 250A max, 230 Nm, ASR On
    */
};

#endif // SCAR_H