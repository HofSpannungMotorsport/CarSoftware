#ifndef IMOTORCONTROLLER_H
#define IMOTORCONTROLLER_H

typedef uint8_t motor_controller_status_t;
enum motor_controller_error_type_t : motor_controller_status_t {
    MOTOR_CONTROLLER_OK = 0x0,
    MOTOR_CONTROLLER_BAD_VAR_GIVEN = 0x1,
    MOTOR_CONTROLLER_ERROR = 0x2
};

class IMotorController {
    public:
        virtual motor_controller_status_t getStatus() = 0;
        virtual void setStatus() = 0;

        virtual void setTorque(float torque) = 0; // from -1.0 to 1.0

        virtual void setReversGearActive(bool active);
};

#endif // IMOTORCONTROLLER_H