#ifndef IMOTORCONTROLLER_H
#define IMOTORCONTROLLER_H

#include "IID.h"

typedef uint8_t motor_controller_status_t;
enum motor_controller_error_type_t : motor_controller_status_t {
    MOTOR_CONTROLLER_OK = 0x0,
    MOTOR_CONTROLLER_BAD_VAR_GIVEN = 0x1,
    MOTOR_CONTROLLER_ERROR = 0x2
};

typedef uint8_t motor_controller_state_t;

class IMotorController : public IID {
    public:
        // Status
        virtual motor_controller_status_t getStatus() = 0;
        virtual void setStatus() = 0;

        // State (-> Status got from the Motor Controller)
        virtual motor_controller_state_t getState() = 0;
        virtual void setState() = 0;

        // Torque
        virtual float getTorque() = 0;
        virtual void setTorque(float torque) = 0; // from 0 to 1.0

        // Speed
        virtual int16_t getSpeed() = 0;

        // Current
        virtual uint8_t getCurrent() = 0;
        virtual uint8_t getCurrentDevice() = 0;

        // Temperature
        virtual uint8_t getMotorTemp() = 0;
        virtual uint8_t getControllerTemp() = 0;
        virtual uint8_t getAirTemp() = 0;

        // Enable
        virtual bool getHardEnabled() = 0;
};

#endif // IMOTORCONTROLLER_H