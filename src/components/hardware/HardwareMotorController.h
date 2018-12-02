#ifndef HARDWAREMOTORCONTROLLER_H
#define HARDWAREMOTORCONTROLLER_H

#include "bamocar-can.h"
#include "../interface/IMotorController.h"

#define STD_SPEED_REFRESH_TIME 40
#define STD_CURRENT_REFRESH_TIME 10
#define STD_CURRENT_DEVICE_REFRESH_TIME 10
#define STD_MOTOR_TEMP_REFRESH_TIME 100
#define STD_CONTROLLER_TEMP_REFRESH_TIME 100
#define STD_AIR_TEMP_REFRESH_TIME 300

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

        void beginCommunication() {
            _bamocar.requestSpeed(STD_SPEED_REFRESH_TIME);
            _bamocar.requestCurrent(STD_CURRENT_REFRESH_TIME);
            _bamocar.requestCurrentDevice(STD_CURRENT_DEVICE_REFRESH_TIME);
            _bamocar.requestMotorTemp(STD_MOTOR_TEMP_REFRESH_TIME);
            _bamocar.requestControllerTemp(STD_CONTROLLER_TEMP_REFRESH_TIME);
            _bamocar.requestAirTemp(STD_AIR_TEMP_REFRESH_TIME);
        }

        virtual motor_controller_status_t getStatus() {
            return _status;
        }

        virtual void setStatus() {
            // No implementation needed
        }

        virtual motor_controller_state_t getState() {
            // The Status of the Motor Controller
            // (not naming it Status because otherwise it conflicts with the Status naming of the other components,
            // so to have a consistant naming and to save errors in "Status", we use the naming State here)
            return _bamocar.getStatus();
        }

        virtual void setState() {
            // No implementation needed
        }

        // To be able to check the Status of the Motor Controller befor starting the whole communication, add a dedicated request method
        void requestState() {
            _bamocar.requestStatus();
        }

        virtual float getTorque() {
            return _bamocar.getTorque();
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
            int16_t torqueValue = (int16_t)((float)setTorqueTo * (float)0x7FFF);
            _bamocar.setTorque(torqueValue);
        }

        virtual int16_t getSpeed() {
            return _bamocar.getSpeed();
        }

        virtual uint8_t getCurrent() {
            return _bamocar.getCurrent();
        }

        virtual uint8_t getCurrentDevice() {
            return _bamocar.getCurrentDevice();
        }

        virtual uint8_t getMotorTemp() {
            return _bamocar.getMotorTemp();
        }

        virtual uint8_t getControllerTemp() {
            return _bamocar.getControllerTemp();
        }

        virtual uint8_t getAirTemp() {
            return _bamocar.getAirTemp();
        }

        virtual bool getHardEnabled() {
            return _bamocar.getHardEnable();
        }

    protected:
        Bamocar _bamocar;

        motor_controller_status_t _status = 0;
};

#endif // HARDWAREMOTORCONTROLLER_H