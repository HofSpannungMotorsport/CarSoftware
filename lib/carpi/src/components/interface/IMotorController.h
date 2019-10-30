#ifndef IMOTORCONTROLLER_H
#define IMOTORCONTROLLER_H

#include "IComponent.h"

typedef uint8_t motor_controller_status_t;
enum motor_controller_error_type_t : motor_controller_status_t {
    MOTOR_CONTROLLER_OK = 0x0,
    MOTOR_CONTROLLER_BAD_VAR_GIVEN = 0x1,
    MOTOR_CONTROLLER_ERROR = 0x2
};

typedef uint8_t motor_controller_state_t;

enum motor_controller_rfe_enable_t : bool {
    MOTOR_CONTROLLER_RFE_DISABLE = false,
    MOTOR_CONTROLLER_RFE_ENABLE = true
};

enum motor_controller_run_enable_t : bool {
    MOTOR_CONTROLLER_RUN_DISABLE = false,
    MOTOR_CONTROLLER_RUN_ENABLE = true
};

class IMotorController : public IComponent {
    public:
        // Status
        virtual motor_controller_status_t getStatus() = 0;
        virtual void setStatus() = 0;

        // State (-> Status got from the Motor Controller)
        virtual motor_controller_state_t getState() = 0;
        virtual void setState() = 0;

        // Torque
        virtual void setTorque(float torque) = 0; // from 0 to 1.0

        // Speed
        virtual float getSpeed() = 0;

        // Current
        virtual float getCurrent() = 0;

        // Temperature
        virtual int16_t getMotorTemp() = 0;
        virtual int16_t getServoTemp() = 0;
        virtual int16_t getAirTemp() = 0;

        // Voltage
        virtual float getDcVoltage() = 0;
        virtual float getDcVoltageGotCount() = 0;

        // Enable setter
        virtual void setRFE(motor_controller_rfe_enable_t state) = 0;
        virtual void setRUN(motor_controller_run_enable_t state) = 0;

        // Method to send interval times
        virtual void beginCommunication() = 0;
};

#endif // IMOTORCONTROLLER_H