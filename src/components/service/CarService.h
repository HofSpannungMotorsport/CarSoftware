#ifndef CARSERVICE_H
#define CARSERVICE_H

#include "mbed.h"
#include <memory>
#include "IService.h"
#include "../interface/IButton.h"
#include "../interface/ILed.h"
#include "../../can/can_ids.h"

#define ERROR_REGISTER_SIZE 64
#define BOOT_ROUTINE_TEST_TIME 2
#define BRAKE_START_THRESHHOLD 0.75

enum car_state_t : uint8_t {
    CAR_OFF = 0x0,
    BOOT = 0x1,
    READY_TO_DRIVE = 0x2,
    ERROR = 0x3
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
                   IPedal* gasPedal, IPedal* brakePedal) {
            _button.reset = buttonReset;
            _button.start = buttonStart;

            _led.red = ledRed;
            _led.yellow = ledYellow;
            _led.green = ledGreen;

            _pedal.gas = gasPedal;
            _pedal.brake = brakePedal;
        }

        virtual void run() {
            if (_button.reset->getStatus() > 0) {
                addError(Error(_calculateComponentId((IID*)_button.reset), _button.reses->getStatus(), ERROR_SYSTEM));
            }

            if (_button.start->getStatus() > 0) {
                addError(Error(_calculateComponentId((IID*)_button.start), _button.start->getStatus(), ERROR_SYSTEM));
            }

            if (!_error.register.empty()) {
                processErrors();
            }
        }

        void addError(Error error) {
            _error.register.push(error);
        }

        void processErrors() {
            while(!_error.register.empty()) {
                Error error;
                _error.register.pop(error);
                if (error.type >= ERROR_UNDEFINED) {
                    if (error.type >= ERROR_SYSTEM) {
                        if (error.type >= ERROR_CRITICAL) {
                            // Critical Error
                            // Red -> Fast Blinking
                            // Green -> Off
                            _led.red->setState(LED_ON);
                            _led.red->setBrightness(1);
                            _led.red->setBlinking(BLINKING_FAST);

                            _led.green->setState(LED_OFF);
                            _state = ERROR;
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
            _startTestOutputs();
            Timer waitTimer;
            waitTimer.reset();
            waitTimer.start();
            // Wait some time to test the outputs for a given time
            while(waitTimer.read() < (float)BOOT_ROUTINE_TEST_TIME) {
                canService.processInbound();
            }
            _stopTestOutputs();


            // Start bootup/calibration
            _state == BOOT;
            _led.yellow->setState(LED_ON);
            _led.green->setState(LED_ON);

            _led.yellow->setBlinking(BLINKING_OFF);
            _led.yellow->setBrightness(0.76);
            _led.green->setBlinking(BLINKING_NORMAL);

            _pedal.gas->setCalibrationStatus(CURRENTLY_CALIBRATING);
            _pedal.brake->setCalibrationStatus(CURRENTLY_CALIBRATING);

            _sendComponentsOverCan();


            // Calibrate pedal until pressed Start once for long time
            while(buttonStart.getStateChanged() || (buttonStart.getState() != LONG_CLICKED)) {
                canService.processInbound();
            }


            // Stop calibration
            _pedal.gas->setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);
            _pedal.brake->setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);
            _sendPedalsOverCan();


            // Wait till the Button got released again
            while(buttonStart.getStateChanged() || (buttonStart.getState() != RELEASED)) {
                canService.processInbound();
            }
            _led.yellow->setState(LED_OFF);
            _sendLedsOverCan();


            // Check for Errors at Calibration
            canService.processInbount();
            if ((_pedal.gas->getStatus() > 0) || (_pedal.brake->getStatus() > 0)) {
                _state = ERROR;
            }


            // If all OK, go into Ready to drive
            if (_state == BOOT) {
                _state = READY_TO_DRIVE;
            } else {
                // If an Error occured, stop continuing and glow Red
                _led.red->setState(LED_ON);
                _led.red->setBlinking(BLINKING_OFF);
                _led.green->setState(LED_OFF);
                _sendLedsOverCan();
                while(1);
            }


            // If no Error, start blinking fast to show ready, but need to start car
            _led.green->setBlinking(BLINKING_FAST);
            _sendLedsOverCan();


            // Wait till Car got started (Brake Pedal + Start Button long press)
            while(buttonStart.getStateChanged() || (buttonStart.getState() != LONG_CLICKED) || (brakePedal.getValue() < BRAKE_START_THRESHHOLD)) {
                canService.processInbound();
            }


            // Stop blinking greed to show car is primed
            _led.green->setBlinking(BLINKING_OFF);
            _sendLedsOverCan();
        }

    private:
        struct _error {
            CircularBuffer<Error, 64, uint8_t> register;
        } _error;

        car_state_t _state = BOOT;

        struct _button {
            IButton* reset, start;
        } _button;

        struct _led {
            ILed* red, yellow, green;
        } _led;

        struct _pedal {
            IPedal* gas, brake;
        } _pedal;

        component_id_t _calculateComponentId(IID* component) {
            component_id_t id = ID::getComponentId(component->getTelegramTypeId(), component->getComponentId());
            return id;
        }

        void _sendLedsOverCan() {
            // LED's
            canService.sendMessage((void*)&_led.red);
            canService.sendMessage((void*)&_led.yellow);
            canService.sendMessage((void*)&_led.green);
        }

        void _sendPedalsOverCan() {
            // Pedals
            canService.sendMessage((void*)&_pedal.gas);
            canService.sendMessage((void*)&_pedal.brake);
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

            _sendComponentsOverCan();
        }

        void _turnOffLed() {
            _led.red->setState(LED_OFF);
            _led.yellow->setState(LED_OFF);
            _led.green->setState(LED_OFF);

            _sendComponentsOverCan();
        }

        void _startTestOutputs() {
            _turnOnLed();
        }

        void _stopTestOutputs() {
            _turnOffLed();
        }
};

#endif // CARSERVICE_H