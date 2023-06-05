#ifndef IMOTORCONTROLLER_H
#define IMOTORCONTROLLER_H

#include "IComponent.h"

typedef uint8_t motor_controller_status_t;
enum motor_controller_error_type_t : motor_controller_status_t
{
    MOTOR_CONTROLLER_OK = 0x0,
    MOTOR_CONTROLLER_BAD_VAR_GIVEN = 0x1,
    MOTOR_CONTROLLER_ERROR = 0x2
};

typedef uint8_t motor_controller_state_t;

enum motor_controller_rfe_enable_t : bool
{
    MOTOR_CONTROLLER_RFE_DISABLE = false,
    MOTOR_CONTROLLER_RFE_ENABLE = true
};

enum motor_controller_run_enable_t : bool
{
    MOTOR_CONTROLLER_RUN_DISABLE = false,
    MOTOR_CONTROLLER_RUN_ENABLE = true
};

class IMotorController : public ICommunication
{
public:
    // Status
    virtual motor_controller_status_t getStatus() = 0;

    // State (-> Status got from the Motor Controller)
    virtual motor_controller_state_t getState() = 0;

    // Torque
    virtual void setTorque(float torque) = 0; // from 0 to 1.0
    virtual void setSentCurrent(uint16_t current) = 0;

    // Speed
    virtual float getSpeed() = 0;
    virtual float getSpeedAge() = 0;

    // Current
    virtual float getCurrent() = 0;
    virtual uint16_t getSentCurrent() = 0;

    // Temperature
    virtual float getMotorTemp() = 0;
    virtual float getMotorTempAge() = 0;
    virtual float getServoTemp() = 0;
    virtual float getServoTempAge() = 0;
    virtual float getAirTemp() = 0;
    virtual float getAirTempAge() = 0;

    // Voltage
    virtual float getDcVoltage() = 0;
    virtual float getDcVoltageAge() = 0;

    // Enable setter
    virtual void setRFE(motor_controller_rfe_enable_t state) = 0;
    virtual void setRUN(motor_controller_run_enable_t state) = 0;

    // Method to send interval times
    virtual void beginCommunication() = 0;
};

#endif // IMOTORCONTROLLER_H