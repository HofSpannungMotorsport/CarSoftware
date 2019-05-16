#ifndef SCAR_H
#define SCAR_H

#include "platform/CircularBuffer.h"
#include "IService.h"
#include "communication/componentIds.h"
#include "components/interface/IButton.h"
#include "components/interface/ILed.h"
#include "components/interface/IBuzzer.h"
#include "components/interface/IMotorController.h"
#include "components/interface/IHvEnabled.h"
#include "components/interface/ISDCard.h"


#define STARTUP_WAIT 0.3 // s wait before system gets started
#define ERROR_REGISTER_SIZE 64 // errors, max: 255
#define STARTUP_ANIMATION_SPEED 0.075 // s between led-changes
#define STARTUP_ANIMATION_PLAYBACKS 2 // times the animation should be played
#define STARTUP_ANIMATION_WAIT_AFTER 0.25 // s wait after animation
#define BRAKE_START_THRESHHOLD 0.60 // %

#define HV_ENABLED_BEEP_TIME 2.2 // s (has to be at least 0.5)

enum car_state_t : uint8_t {
    CAR_OFF = 0x0,
    BOOT = 0x1,
    ALMOST_READY_TO_DRIVE = 0x2,
    READY_TO_DRIVE = 0x3,
    CAR_ERROR = 0x4
};

enum error_type_t : uint8_t {
    ERROR_NO = 0x0,
    ERROR_UNDEFINED = 0x1,
    ERROR_ISSUE = 0x2,
    ERROR_SYSTEM = 0x3, // buffer-overflow...
    ERROR_CRITICAL = 0x4 // Pedal-Error -> not ready to drive anymore
};

class Error {
    public:
        Error(){}
        Error(id_component_t ComponentId, uint8_t Code, error_type_t Type)
            : componentId(ComponentId), code(Code), type(Type) {}

        id_component_t componentId = 0;
        uint8_t code = 0;
        error_type_t type = ERROR_NO;
};

class SCar : public IService {
    public:
        SCar(IButton* buttonReset, IButton* buttonStart,
             ILed* ledRed, ILed* ledYellow, ILed* ledGreen,
             IPedal* gasPedal, IPedal* brakePedal,
             IBuzzer* buzzer,
             IMotorController* motorController,
             IHvEnabled* hvEnabled,
             ISDCard* sdCard) {
            _button.reset = buttonReset;
            _button.start = buttonStart;

            _led.red = ledRed;
            _led.yellow = ledYellow;
            _led.green = ledGreen;

            _pedal.gas = gasPedal;
            _pedal.brake = brakePedal;

            _buzzer = buzzer;

            _motorController = motorController;

            _hvEnabled = hvEnabled;

            _sdCard = sdCard;
        }

        virtual void run() {
            _checkHvEnabled();

            _checkInput();

            if (_button.reset->getStatus() > 0) {
                addError(Error(_calculateComponentId((IComponent*)_button.reset), _button.reset->getStatus(), ERROR_SYSTEM));
            }

            if (_button.start->getStatus() > 0) {
                addError(Error(_calculateComponentId((IComponent*)_button.start), _button.start->getStatus(), ERROR_SYSTEM));
            }

            if (!(_errorRegister.empty())) {
                processErrors();
            }
        }

        void addError(Error error) {
            _errorRegister.push(error);
        }

        void processErrors() {
            while(!_errorRegister.empty()) {
                Error error;
                _errorRegister.pop(error);
                pcSerial.printf("[SCar]@processErrors: Got Error at 0x%x with error code 0x%x and error type 0x%x !\n", error.componentId, error.code, error.type);
                if (error.type >= ERROR_UNDEFINED) {
                    _resetLeds();
                    if (error.type >= ERROR_SYSTEM) {
                        if (error.type >= ERROR_CRITICAL) {
                            // Critical Error
                            // Red -> Fast Blinking
                            // Green -> Off
                            _led.red->setState(LED_ON);
                            _led.red->setBrightness(1);
                            _led.red->setBlinking(BLINKING_FAST);

                            _led.green->setState(LED_OFF);
                            _state = CAR_ERROR;

                            _motorController->setRUN(MOTOR_CONTROLLER_RUN_DISABLE);
                            _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);
                        } else {
                            // System Error
                            // Yellow -> Fast Blinking
                            _led.yellow->setState(LED_ON);
                            _led.yellow->setBrightness(1);
                            _led.yellow->setBlinking(BLINKING_FAST);
                        }
                    } else {
                        // Issue / Undefined
                        // Yellow -> On @ 70%
                        _led.yellow->setState(LED_ON);
                        _led.yellow->setBrightness(0.51);
                        _led.yellow->setBlinking(BLINKING_OFF);
                    }
                }
            }
        }

        car_state_t getState() {
            return _state;
        }

        void startUp() {
            wait(STARTUP_WAIT);

            for (uint8_t i = 0; i < STARTUP_ANIMATION_PLAYBACKS; i++) {
                _startupAnimationUp();
                _startupAnimationDown();
            }

            wait(STARTUP_ANIMATION_WAIT_AFTER);

            // Turn on red LED while HV-Circuite is off
            _resetLeds();
            _led.red->setState(LED_ON);

            // [QF]
            while(!(_hvEnabled->read())) {
                wait(0.1);
            }

           	_resetLeds();

            wait(0.1);

            _calibratePedals();

            _checkHvEnabled();
            processErrors();
            if (_state != ALMOST_READY_TO_DRIVE) {
                // If an Error occured, stop continuing and glow Red
                // Red   -> Blinking Fast
                // Green -> Off
                _led.red->setState(LED_ON);
                _led.red->setBlinking(BLINKING_FAST);
                _led.green->setState(LED_OFF);
                while(1) {
                    wait(100);
                }
            }
        }

    private:
        CircularBuffer<Error, ERROR_REGISTER_SIZE, uint8_t> _errorRegister;

        car_state_t _state = CAR_OFF;

        struct _button {
            IButton* reset;
            IButton* start;
        } _button;

        struct _led {
            ILed* red;
            ILed* yellow;
            ILed* green;
        } _led;

        struct _pedal {
            IPedal* gas;
            IPedal* brake;
        } _pedal;

        IBuzzer* _buzzer;

        IMotorController* _motorController;

        IHvEnabled* _hvEnabled;

        ISDCard* _sdCard;

        id_component_t _calculateComponentId(IComponent* component) {
            id_component_t id = component->getComponentId();
            return id;
        }

        void _startupAnimationUp() {
            _resetLeds();
            _led.red->setState(LED_ON);

            wait(STARTUP_ANIMATION_SPEED);

            _led.yellow->setState(LED_ON);

            wait(STARTUP_ANIMATION_SPEED);

            _led.green->setState(LED_ON);
            _led.red->setState(LED_OFF);

            wait(STARTUP_ANIMATION_SPEED);

            _led.yellow->setState(LED_OFF);

            wait(STARTUP_ANIMATION_SPEED);

            _led.green->setState(LED_OFF);

            wait(STARTUP_ANIMATION_SPEED);
        }

        void _startupAnimationDown() {
            _resetLeds();
            _led.green->setState(LED_ON);

            wait(STARTUP_ANIMATION_SPEED);

            _led.yellow->setState(LED_ON);

            wait(STARTUP_ANIMATION_SPEED);

            _led.red->setState(LED_ON);
            _led.green->setState(LED_OFF);

            wait(STARTUP_ANIMATION_SPEED);

            _led.yellow->setState(LED_OFF);

            wait(STARTUP_ANIMATION_SPEED);

            _led.red->setState(LED_OFF);

            wait(STARTUP_ANIMATION_SPEED);
        }

        void _resetLeds() {
            _led.red->setBrightness(1);
            _led.yellow->setBrightness(1);
            _led.green->setBrightness(1);

            _led.red->setState(LED_OFF);
            _led.yellow->setState(LED_OFF);
            _led.green->setState(LED_OFF);

            _led.red->setBlinking(BLINKING_OFF);
            _led.yellow->setBlinking(BLINKING_OFF);
            _led.green->setBlinking(BLINKING_OFF);
        }

        void _pedalError(IPedal* sensorId) {
            addError(Error(sensorId->getComponentId(), sensorId->getStatus(), ERROR_CRITICAL));
        }

        void _checkHvEnabled() {
            // [QF]
            if (!(_hvEnabled->read())) {
                addError(Error(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_HV_ENABLED), 0x1, ERROR_CRITICAL));
            }
        }

        void _calibratePedals() {
            // Start bootup/calibration
            // Yellow -> On
            // Green  -> Normal Blinking
            _state = BOOT;
            _resetLeds();
            _led.yellow->setState(LED_ON);
            _led.green->setState(LED_ON);

            _led.yellow->setBlinking(BLINKING_OFF);
            _led.yellow->setBrightness(0.75);
            _led.green->setBlinking(BLINKING_NORMAL);

            _pedal.gas->setCalibrationStatus(CURRENTLY_CALIBRATING);
            _pedal.brake->setCalibrationStatus(CURRENTLY_CALIBRATING);


            // Calibrate pedal until pressed Start once for long time
            while(_button.start->getState() != LONG_CLICKED) {
                wait(0.1);
            }


            // Stop calibration
            _pedal.gas->setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);
            _pedal.brake->setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);

            wait(0.1);
            _led.yellow->setState(LED_OFF);



            // Wait till the Button got released again
            // Yellow -> Off
            while(_button.start->getState() != NOT_PRESSED) {
                wait(0.1);
            }

            wait(0.1);


            // Check for Errors at Calibration
            if (_pedal.gas->getStatus() > 0) {
                _pedalError(_pedal.gas);
            }

            if (_pedal.brake->getStatus() > 0) {
                _pedalError(_pedal.brake);
            }

            processErrors();


            // If all OK, go into Almost Ready to drive
            if (_state == BOOT) {
                _state = ALMOST_READY_TO_DRIVE;
            } else {
                // If an Error occured, stop continuing and glow Red
                // Red   -> Blinking Slow
                // Green -> Off
                _resetLeds();
                _led.red->setState(LED_ON);
                _led.red->setBlinking(BLINKING_SLOW);
                while(1) {
                    wait(100);
                }
            }


            // If no Error, start blinking fast to show ready, but need to start car
            // Green -> Fast Blinking
            _resetLeds();
            _led.green->setState(LED_ON);
            _led.green->setBlinking(BLINKING_FAST);

            wait(0.1);
        }

        void _checkInput() {
            if (_button.reset->getStateChanged()) {
                if (_button.reset->getState() == PRESSED) {
                    if (_state == READY_TO_DRIVE) {
                        // Disable RFE and RUN
                        _motorController->setRUN(MOTOR_CONTROLLER_RUN_DISABLE);
                        _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);

                        _state = ALMOST_READY_TO_DRIVE;

                        _led.green->setBlinking(BLINKING_FAST);
                    }
                } else if (_button.reset->getState() == LONG_CLICKED) {
                    if (_state != READY_TO_DRIVE) {
                        // ReCalibrate the Pedals
                        _calibratePedals();
                    }
                }
            }

            // [QF]
            if (_state == ALMOST_READY_TO_DRIVE) {
                if ((_button.start->getState() == LONG_CLICKED) && (_pedal.brake->getValue() >= BRAKE_START_THRESHHOLD)) {
                    // Optimize later!!
                    // [il]
                    // Set RFE enable
                    _motorController->setRFE(MOTOR_CONTROLLER_RFE_ENABLE);

                    // [QF]
                    //Start beeping to signalize car is started
                    _buzzer->setBeep(BUZZER_MONO_TONE);
                    _buzzer->setState(BUZZER_ON);

                    // Wait the set Time until enabling RUN
                    Timer waitTimer;
                    waitTimer.reset();
                    waitTimer.start();
                    do {
                        _checkHvEnabled();
                        processErrors();
                        if (_state != ALMOST_READY_TO_DRIVE) {
                            _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);
                            // If an Error occured, stop continuing and glow Red
                            // Red   -> Blinking Normal
                            // Green -> Off
                            _resetLeds();
                            _led.red->setState(LED_ON);
                            _led.red->setBlinking(BLINKING_NORMAL);

                            // Stop beeping!
                            _buzzer->setState(BUZZER_OFF);

                            while(1) {
                                wait(100);
                            }
                        }
                    } while (waitTimer.read() < (float)HV_ENABLED_BEEP_TIME);

                    // Set RUN enable if no Error
                    _checkHvEnabled();
                    processErrors();

                    // Stop beeping!
                    _buzzer->setState(BUZZER_OFF);

                    if (_state == ALMOST_READY_TO_DRIVE) {
                        _motorController->setRUN(MOTOR_CONTROLLER_RUN_ENABLE);
                    } else {
                        _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);
                        // If an Error occured, stop continuing and glow Red
                        // Red   -> Blinking Normal
                        // Green -> Off
                        _resetLeds();
                        _led.red->setState(LED_ON);
                        _led.red->setBlinking(BLINKING_NORMAL);
                        while(1);
                    }

                    // Set car ready to drive (-> pressing the gas-pedal will move the car -> fun)
                    _state = READY_TO_DRIVE;

                    // Stop blinking greed to show car is primed
                    _resetLeds();
                    _led.green->setState(LED_ON);
                    wait(0.1);
                }
            }
        }
};

#endif // SCAR_H