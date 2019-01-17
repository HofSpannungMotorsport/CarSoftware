#ifndef CARSERVICE_H
#define CARSERVICE_H

#include "mbed.h"
#include <memory>
#include "IService.h"
#include "../interface/IButton.h"
#include "../interface/ILed.h"
#include "../interface/IBuzzer.h"
#include "../interface/IMotorController.h"
#include "../../can/can_ids.h"

#define ERROR_REGISTER_SIZE 64 // errors, max: 255
#define BOOT_ROUTINE_TEST_TIME 2 // s
#define BRAKE_START_THRESHHOLD 0.75 // %

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
        Error(component_id_t ComponentId, uint8_t Code, error_type_t Type)
            : componentId(ComponentId), code(Code), type(Type) {}

        component_id_t componentId;
        uint8_t code = 0;
        error_type_t type;
};

class CarService : public IService {
    public:
        CarService(IButton* buttonReset, IButton* buttonStart,
                   ILed* ledRed, ILed* ledYellow, ILed* ledGreen,
                   IPedal* gasPedal, IPedal* brakePedal,
                   IBuzzer* buzzer,
                   IMotorController* motorController,
                   DigitalIn &hvEnabled)
            : _hvEnabled(hvEnabled) {
            _button.reset = buttonReset;
            _button.start = buttonStart;

            _led.red = ledRed;
            _led.yellow = ledYellow;
            _led.green = ledGreen;

            _pedal.gas = gasPedal;
            _pedal.brake = brakePedal;

            _buzzer = buzzer;

            _motorController = motorController;
        }

        virtual void run() {
            _checkHvEnabled();

            if (_button.reset->getStatus() > 0) {
                addError(Error(_calculateComponentId((IID*)_button.reset), _button.reset->getStatus(), ERROR_SYSTEM));
            }

            if (_button.start->getStatus() > 0) {
                addError(Error(_calculateComponentId((IID*)_button.start), _button.start->getStatus(), ERROR_SYSTEM));
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

                    _sendLedsOverCan();
                }
            }
        }

        car_state_t getState() {
            return _state;
        }

        void startUp() {
            wait(2);
            _startTestOutputs();
            Timer waitTimer;
            waitTimer.reset();
            waitTimer.start();
            // Wait some time to test the outputs for a given time
            while(waitTimer.read() < (float)BOOT_ROUTINE_TEST_TIME) {
                canService.processInbound();
            }
            _stopTestOutputs();

            // Turn on red LED while HV-Circuite is off
            _resetLeds();
            _led.red->setState(LED_ON);
            _sendLedsOverCan();

            /*
            while(!_hvEnabled) {
                canService.processInbound();
            }
            */

           	_resetLeds();
            _led.red->setState(LED_OFF);
            _sendLedsOverCan();

            wait(0.1);

            // Start bootup/calibration
            // Yellow -> On
            // Green  -> Normal Blinking
            _state = BOOT;
            _resetLeds();
            _led.yellow->setState(LED_ON);
            _led.green->setState(LED_ON);

            _led.yellow->setBlinking(BLINKING_OFF);
            _led.yellow->setBrightness(0.76);
            _led.green->setBlinking(BLINKING_NORMAL);

            _pedal.gas->setCalibrationStatus(CURRENTLY_CALIBRATING);
            _pedal.brake->setCalibrationStatus(CURRENTLY_CALIBRATING);

            _sendComponentsOverCan();


            // Calibrate pedal until pressed Start once for long time
            while(_button.start->getState() != LONG_CLICKED) {
                canService.processInbound();
            }


            // Stop calibration
            _pedal.gas->setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);
            _pedal.brake->setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);
            _sendPedalsOverCan();


            // Wait till the Button got released again
            // Yellow -> Off
            while(_button.start->getState() != NOT_PRESSED) {
                canService.processInbound();
            }
            _led.yellow->setState(LED_OFF);
            _sendLedsOverCan();

            wait(0.1);


            // Check for Errors at Calibration
            canService.processInbound();
            if ((_pedal.gas->getStatus() > 0) || (_pedal.brake->getStatus() > 0)) {
                _state = CAR_ERROR;
            }


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
                _sendLedsOverCan();
                while(1) {
                    wait(100);
                }
            }


            // If no Error, start blinking fast to show ready, but need to start car
            // Green -> Fast Blinking
            _resetLeds();
            _led.green->setState(LED_ON);
            _led.green->setBlinking(BLINKING_FAST);
            _sendLedsOverCan();

            wait(0.1);


            // Wait till Car got started (Brake Pedal + Start Button long press) and check for Errors/HV disabled
            do {
                canService.processInbound();
                _checkHvEnabled();
                processErrors();
                if (_state != ALMOST_READY_TO_DRIVE) {
                    // If an Error occured, stop continuing and glow Red
                    // Red   -> Blinking Fast
                    // Green -> Off
                    _led.red->setState(LED_ON);
                    _led.red->setBlinking(BLINKING_FAST);
                    _led.green->setState(LED_OFF);
                    _sendLedsOverCan();
                    while(1) {
                        wait(100);
                    }
                }
            } while ((_button.start->getState() != LONG_CLICKED) || (_pedal.brake->getValue() < BRAKE_START_THRESHHOLD));

            // Optimize later!!
            // [il]
            // Set RFE enable
            _motorController->setRFE(MOTOR_CONTROLLER_RFE_ENABLE);

            //Start beeping to signalize car is started
            _buzzer->setBeep(BUZZER_MONO_TONE);
            _buzzer->setState(BUZZER_ON);

            // Wait the set Time until enabling RUN
            waitTimer.reset();
            waitTimer.start();
            do {
                _checkHvEnabled();
                processErrors();
                canService.processInbound();
                if (_state != ALMOST_READY_TO_DRIVE) {
                    _motorController->setRFE(MOTOR_CONTROLLER_RFE_DISABLE);
                    // If an Error occured, stop continuing and glow Red
                    // Red   -> Blinking Normal
                    // Green -> Off
                    _resetLeds();
                    _led.red->setState(LED_ON);
                    _led.red->setBlinking(BLINKING_NORMAL);
                    _sendLedsOverCan();
                    while(1) {
                        wait(100);
                    }
                }
            } while (waitTimer.read() < (float)HV_ENABLED_BEEP_TIME);

            // Set RUN enable if no Error
            canService.processInbound();
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
                _sendLedsOverCan();
                while(1);
            }

            // Set car ready to drive (-> pressing the gas-pedal will move the car -> fun)
            _state = READY_TO_DRIVE;
            _readyToDriveFor.reset();
            _readyToDriveFor.start();

            // Stop blinking greed to show car is primed
            _resetLeds();
            _led.green->setState(LED_ON);
            _sendLedsOverCan();
            wait(0.1);
        }

    private:
        CircularBuffer<Error, ERROR_REGISTER_SIZE, uint8_t> _errorRegister;

        car_state_t _state = CAR_OFF;
        Timer _readyToDriveFor;

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

        DigitalIn &_hvEnabled;

        component_id_t _calculateComponentId(IID* component) {
            component_id_t id = ID::getComponentId(component->getTelegramTypeId(), component->getComponentId());
            return id;
        }

        void _sendLedsOverCan() {
            // LED's
            canService.sendMessage((void*)_led.red);
            canService.sendMessage((void*)_led.yellow);
            canService.sendMessage((void*)_led.green);
        }

        void _sendPedalsOverCan() {
            // Pedals
            canService.sendMessage((void*)_pedal.gas);
            canService.sendMessage((void*)_pedal.brake);
        }

        void _sendComponentsOverCan() {
            _sendLedsOverCan();
            _sendPedalsOverCan();
        }

        void _turnOnLed() {
            _led.red->setBrightness(1);
            _led.yellow->setBrightness(1);
            _led.green->setBrightness(1);

            _led.red->setState(LED_ON);
            _led.yellow->setState(LED_ON);
            _led.green->setState(LED_ON);

            _sendLedsOverCan();
        }

        void _turnOffLed() {
            _led.red->setState(LED_OFF);
            _led.yellow->setState(LED_OFF);
            _led.green->setState(LED_OFF);

            _sendLedsOverCan();
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

        void _startTestOutputs() {
            _turnOnLed();
        }

        void _stopTestOutputs() {
            _turnOffLed();
        }

        void _checkHvEnabled() {
            /*
            if (!_hvEnabled) {
                addError(Error(ID::getComponentId(SYSTEM, SYSTEM_HV_ENABLED), 0x1, ERROR_CRITICAL));
            }
            */
        }
};

#endif // CARSERVICE_H