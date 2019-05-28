#ifndef SCAR_H
#define SCAR_H

#include <string>

#include "IService.h"
#include "communication/componentIds.h"
#include "components/interface/IButton.h"
#include "components/interface/ILed.h"
#include "components/interface/IBuzzer.h"
#include "components/interface/IMotorController.h"
#include "components/interface/IHvEnabled.h"
#include "components/interface/ISDCard.h"
#include "components/interface/IAlive.h"
#include "communication/Sync.h"


#define ERROR_REGISTER_SIZE 64 // errors, max: 255
#define STARTUP_ANIMATION_SPEED 0.075 // s between led-changes
#define STARTUP_ANIMATION_PLAYBACKS 2 // times the animation should be played
#define STARTUP_ANIMATION_WAIT_AFTER 0.25 // s wait after animation
#define BRAKE_START_THRESHHOLD 0.60 // %

#define HV_ENABLED_BEEP_TIME 2.2 // s (has to be at least 0.5)

enum car_state_t : uint8_t {
    CAR_OFF = 0x0,
    CALIBRATION = 0x1,
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
        SCar(Sync &syncer,
             IButton* buttonReset, IButton* buttonStart,
             ILed* ledRed, ILed* ledYellow, ILed* ledGreen,
             IPedal* gasPedal, IPedal* brakePedal,
             IBuzzer* buzzer,
             IMotorController* motorController,
             IHvEnabled* hvEnabled,
             ISDCard* sdCard,
             IAlive* pedalAlive, IAlive* dashboardAlive, IAlive* masterAlive)
             : _syncer(syncer) {
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

            _pedalAlive = pedalAlive;
            _dashboardAlive = dashboardAlive;
            _masterAlive = masterAlive;
        }

        virtual void run() {
            _checkHvEnabled();
            _checkAlive();
            _checkButtonStatus();
            _checkSDCard();

            processErrors();

            _checkInput();
        }

        void addError(Error error) {
            _errorRegister.push(error);
        }

        void processErrors() {
            Error error;
            while(_errorRegister.pop(error)) {
                // Send error over Serial to PC
                pcSerial.printf("[SCar]@processErrors: Got Error at 0x%x with error code 0x%x and error type 0x%x !\n", error.componentId, error.code, error.type);

                // Document raw error to SD
                char buffer[32];
                sprintf(buffer, "0x%x;0x%x;0x%x", error.componentId, error.code, error.type);
                string errorSDLog = buffer;
                _sdCard->write(*_sdCard, SD_LOG_ID_SCAR_ERROR, errorSDLog);

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
                            setState(CAR_ERROR);

                            _motorController->setRUN(MOTOR_CONTROLLER_RUN_DISABLE);
                            _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);
                        } else {
                            // System Error
                            // Yellow -> Fast Blinking
                            _led.yellow->setState(LED_ON);
                            _led.yellow->setBrightness(1);
                            _led.yellow->setBlinking(BLINKING_OFF);
                        }
                    } else {
                        // Issue / Undefined
                        // Yellow -> On @ 25%
                        // But only if not already on (probably because of System Error)
                        if (_led.yellow->getBrightness() < 0.25 || _led.yellow->getState() != LED_ON) {
                            _led.yellow->setState(LED_ON);
                            _led.yellow->setBrightness(0.25);
                            _led.yellow->setBlinking(BLINKING_OFF);
                        }
                    }
                }
            }
        }

        car_state_t getState() {
            return _state;
        }

        void setState(car_state_t state) {
            _state = state;
            string newState = to_string(_state);
            _sdCard->write(*_sdCard, SD_LOG_ID_SCAR_STATE, newState);
        }

        void startUp() {
            for (uint8_t i = 0; i < STARTUP_ANIMATION_PLAYBACKS; i++) {
                _startupAnimationUp();
                _startupAnimationDown();
            }

            wait(STARTUP_ANIMATION_WAIT_AFTER);

            // Turn on red LED while HV-Circuite is off
            _resetLeds();
            _led.red->setState(LED_ON);
            _waitForSent();

            wait(0.1);

            // [QF]
            while(!(_hvEnabled->read())) {
                _syncer.run();
                _checkAlive();
                processErrors();
                wait(0.1);
            }

           	_resetLeds();

            wait(0.1);

            _calibratePedals();

            _checkHvEnabled();
            processErrors();
            if (getState() != ALMOST_READY_TO_DRIVE) {
                // If an Error occured, stop continuing and glow Red
                // Red   -> Blinking Fast
                // Green -> Off
                _led.red->setState(LED_ON);
                _led.red->setBlinking(BLINKING_FAST);
                _led.green->setState(LED_OFF);
                _waitForSent();
                while(true) {
                    _syncer.run();
                    wait(0.1);
                }
            }
        }

    #ifdef TESTING_MODE
    protected:
        SCar(Sync &syncer) : _syncer(syncer) {} // Only use for testing outside of the car!
    #else
    private:
    #endif
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

        Sync &_syncer;
        IBuzzer* _buzzer;
        IMotorController* _motorController;
        IHvEnabled* _hvEnabled;
        ISDCard* _sdCard;

        IAlive* _pedalAlive;
        IAlive* _dashboardAlive;
        IAlive* _masterAlive;

        id_component_t _calculateComponentId(IComponent* component) {
            id_component_t id = component->getComponentId();
            return id;
        }

        void _waitForSent() {
            while(_syncer.messageInQueue()) {
                _syncer.run();
            }
        }

        void _startupAnimationUp() {
            _resetLeds();
            _led.red->setState(LED_ON);
            _waitForSent();

            wait(STARTUP_ANIMATION_SPEED);

            _led.yellow->setState(LED_ON);
            _waitForSent();

            wait(STARTUP_ANIMATION_SPEED);

            _led.green->setState(LED_ON);
            _led.red->setState(LED_OFF);
            _waitForSent();

            wait(STARTUP_ANIMATION_SPEED);

            _led.yellow->setState(LED_OFF);
            _waitForSent();

            wait(STARTUP_ANIMATION_SPEED);

            _led.green->setState(LED_OFF);
            _waitForSent();

            wait(STARTUP_ANIMATION_SPEED);
        }

        void _startupAnimationDown() {
            _resetLeds();
            _led.green->setState(LED_ON);
            _waitForSent();

            wait(STARTUP_ANIMATION_SPEED);

            _led.yellow->setState(LED_ON);
            _waitForSent();

            wait(STARTUP_ANIMATION_SPEED);

            _led.red->setState(LED_ON);
            _led.green->setState(LED_OFF);
            _waitForSent();

            wait(STARTUP_ANIMATION_SPEED);

            _led.yellow->setState(LED_OFF);
            _waitForSent();

            wait(STARTUP_ANIMATION_SPEED);

            _led.red->setState(LED_OFF);
            _waitForSent();

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

            _waitForSent();
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

        void _checkAlive() {
            // [QF]
            bool anyControllerDead = false;

            if (!(_pedalAlive->getAlive())) {
                addError(Error(_calculateComponentId((IComponent*)_pedalAlive), _pedalAlive->getStatus(), ERROR_CRITICAL));
                anyControllerDead = true;
            }

            if (!(_dashboardAlive->getAlive())) {
                addError(Error(_calculateComponentId((IComponent*)_dashboardAlive), _dashboardAlive->getStatus(), ERROR_CRITICAL));
                anyControllerDead = true;
            }

            if (!(_masterAlive->getAlive())) {
                addError(Error(_calculateComponentId((IComponent*)_masterAlive), _masterAlive->getStatus(), ERROR_CRITICAL));
                anyControllerDead = true;
            }

            if (anyControllerDead) {
                _buzzer->setBeep(BUZZER_BEEP_FAST_HIGH_LOW);
                _buzzer->setState(BUZZER_ON);
            } else {
                _buzzer->setBeep(BUZZER_MONO_TONE);
                _buzzer->setState(BUZZER_OFF);
            }
        }

        void _checkButtonStatus() {
            if (_button.reset->getStatus() > 0) {
                addError(Error(_calculateComponentId((IComponent*)_button.reset), _button.reset->getStatus(), ERROR_ISSUE));
            }

            if (_button.start->getStatus() > 0) {
                addError(Error(_calculateComponentId((IComponent*)_button.start), _button.start->getStatus(), ERROR_ISSUE));
            }
        }

        void _checkSDCard() {
            if (_sdCard->getStatus() > 0) {
                addError(Error(_sdCard->getComponentId(), _sdCard->getStatus(), ERROR_SYSTEM));
            }
        }

        void _calibratePedals() {
            // Start bootup/calibration
            // Yellow -> On
            // Green  -> Normal Blinking
            setState(CALIBRATION);
            _resetLeds();
            _led.yellow->setState(LED_ON);
            _led.green->setState(LED_ON);

            _led.yellow->setBlinking(BLINKING_OFF);
            _led.yellow->setBrightness(0.75);
            _led.green->setBlinking(BLINKING_NORMAL);

            _pedal.gas->setCalibrationStatus(CURRENTLY_CALIBRATING);
            _pedal.brake->setCalibrationStatus(CURRENTLY_CALIBRATING);

            _waitForSent();


            // Calibrate pedal until pressed Start once for long time
            while(_button.start->getState() != LONG_CLICKED) {
                _syncer.run();
                _checkAlive();
                wait(0.1);
            }


            // Stop calibration
            _pedal.gas->setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);
            _pedal.brake->setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);
            _waitForSent();

            wait(0.1);
            _led.yellow->setState(LED_OFF);
            _waitForSent();



            // Wait till the Button got released again
            // Yellow -> Off
            while(_button.start->getState() != NOT_PRESSED) {
                _syncer.run();
                wait(0.1);
            }

            wait(0.1);

            _syncer.run();
            _checkAlive();

            // Check for Errors at Calibration
            if (_pedal.gas->getStatus() > 0) {
                _pedalError(_pedal.gas);
            }

            if (_pedal.brake->getStatus() > 0) {
                _pedalError(_pedal.brake);
            }

            processErrors();


            // If all OK, go into Almost Ready to drive
            if (getState() == CALIBRATION) {
                setState(ALMOST_READY_TO_DRIVE);
            } else {
                // If an Error occured, stop continuing and glow Red
                // Red   -> Blinking Slow
                // Green -> Off
                _resetLeds();
                _led.red->setState(LED_ON);
                _led.red->setBlinking(BLINKING_SLOW);
                _waitForSent();
                while(true) {
                    _syncer.run();
                    wait(0.1);
                }
            }


            // If no Error, start blinking fast to show ready, but need to start car
            // Green -> Fast Blinking
            _resetLeds();
            _led.green->setState(LED_ON);
            _led.green->setBlinking(BLINKING_FAST);
            _waitForSent();

            wait(0.1);
        }

        void _checkInput() {
            if (_button.reset->getStateChanged()) {
                if (_button.reset->getState() == PRESSED) {
                    if (getState() == READY_TO_DRIVE) {
                        // Disable RFE and RUN
                        _motorController->setRUN(MOTOR_CONTROLLER_RUN_DISABLE);
                        _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);

                        setState(ALMOST_READY_TO_DRIVE);

                        _led.green->setBlinking(BLINKING_FAST);
                    }
                } else if (_button.reset->getState() == LONG_CLICKED) {
                    if (getState() != READY_TO_DRIVE) {
                        // ReCalibrate the Pedals
                        _calibratePedals();
                    }
                }
            }

            // [QF]
            if (getState() == ALMOST_READY_TO_DRIVE) {
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
                        if (getState() != ALMOST_READY_TO_DRIVE) {
                            _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);
                            // If an Error occured, stop continuing and glow Red
                            // Red   -> Blinking Normal
                            // Green -> Off
                            _resetLeds();
                            _led.red->setState(LED_ON);
                            _led.red->setBlinking(BLINKING_NORMAL);
                            _waitForSent();

                            // Stop beeping!
                            _buzzer->setState(BUZZER_OFF);

                            while(true) {
                                _syncer.run();
                                wait(0.1);
                            }
                        }
                    } while (waitTimer.read() < (float)HV_ENABLED_BEEP_TIME);

                    // Set RUN enable if no Error
                    _checkHvEnabled();
                    processErrors();

                    // Stop beeping!
                    _buzzer->setState(BUZZER_OFF);

                    if (getState() == ALMOST_READY_TO_DRIVE) {
                        _motorController->setRUN(MOTOR_CONTROLLER_RUN_ENABLE);
                    } else {
                        _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);
                        // If an Error occured, stop continuing and glow Red
                        // Red   -> Blinking Normal
                        // Green -> Off
                        _resetLeds();
                        _led.red->setState(LED_ON);
                        _led.red->setBlinking(BLINKING_NORMAL);
                        _waitForSent();

                        while(true) {
                            _syncer.run();
                            wait(0.1);
                        }
                    }

                    // Set car ready to drive (-> pressing the gas-pedal will move the car -> fun)
                    setState(READY_TO_DRIVE);

                    // Stop blinking greed to show car is primed
                    _resetLeds();
                    _led.green->setState(LED_ON);
                    _waitForSent();

                    wait(0.1);
                }
            }
        }
};

#endif // SCAR_H