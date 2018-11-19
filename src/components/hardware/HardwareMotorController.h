#ifndef HARDWAREMOTORCONTROLLER_H
#define HARDWAREMOTORCONTROLLER_H

#include "bamocar-can.h"
#include "../interface/IMotorController.h"

#define MAX_REVERSE_TORQUE 0.1 // %

class HardwareMotorController : public IMotorController {
    public:
        HardwareMotorController(PinName canRD, PinName canTD)
            : _bamocar(canRD, canTD) {
            _telegramTypeId = MOTOR;
            _objectType = HARDWARE_OBJECT;
        }

        HardwareMotorController(PinName canRD, PinName canTD, can_component_t componentId)
            : HardwareMotorController(canRD, canTD) {
            _componentId = componentId;
        }
        
        virtual motor_controller_status_t getStatus() {
            return _status;
        }

        virtual void setStatus() {
            // No implementation needed
        }

        virtual void setTorque(float torque) {
            float setTorqueTo = torque;

            // Check if out of bounds (0.0 - 1.0)
            if (torque > 1.0) {
                if (torque > 1.1) {
                    // Given Value too far outside of our bounds (so not only float error)
                    motor_controller_error_type_t badValError = MOTOR_CONTROLLER_BAD_VAR_GIVEN;
                    _status = badValError;
                    setTorqueTo = 0;
                } else {
                    // Else, set to boundary
                    setTorqueTo = 1.0;
                }
            } else if (torque < 0) {
                // A negative Value would result in a reverse-rotating motor -> not allowed!
                if (torque < (-0.1)) {
                    motor_controller_error_type_t badValError = MOTOR_CONTROLLER_BAD_VAR_GIVEN;
                    _status = badValError;
                    setTorqueTo = 0;
                } else {
                    setTorqueTo = 0;
                }
            }

            // Set the torque to a 16-bit integer
            int16_t torqueValue = (int16_t)(setTorqueTo * (float)0x7FFF);
            _bamocar.setTorque(torqueValue);
        }

    protected:
        Bamocar _bamocar;

        motor_controller_status_t _status = 0;
};

#endif // HARDWAREMOTORCONTROLLER_H