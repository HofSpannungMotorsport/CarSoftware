#ifndef CARSERVICE_H
#define CARSERVICE_H

#include "mbed.h"
#include <memory>
#include "IService.h"
#include "../interface/IButton.h"
#include "../interface/ILed.h"
#include "../../can/can_ids.h"

#define ERROR_REGISTER_SIZE 64

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
            : componentId(ComponentId), code(Code) {}

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
                if (error.type >= ERROR_ISSUE) {
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
                        }
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
                    _led.yellow->setBrightness(0.7);
                    _led.yellow->setBlinking(BLINKING_OFF);
                }
            }
        }

        car_state_t getState() {
            return _state;
        }

        void testLed() {
            _led.red->setBrightness(1);
            _led.yellow->setBrightness(1);
            _led.green->setBrightness(1);

            _led.red->setState(LED_ON);
            _led.yellow->setState(LED_ON);
            _led.green->setState(LED_ON);
        }

        void testOutputs() {
            testLed();
        }

        void boot() {
            _led.red->setState(LED_OFF);
            _led.yellow->setState(LED_OFF);
            _led.green->setState(LED_ON);

            _led.green->setBlinking(BLINKING_NORMAL);

            _state == BOOT;

            _pedal.gas->setCalibrationStatus(CURRENTLY_CALIBRATING);
            _pedal.brake->setCalibrationStatus(CURRENTLY_CALIBRATING);
        }

        void setCarReady() {
            if (_state == BOOT) {
                _led.green->setBlinking(BLINKING_OFF);

                _pedal.gas->setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);
                _pedal.brake->setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);

                _state = READY_TO_DRIVE;
            }
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
};

#endif // CARSERVICE_H