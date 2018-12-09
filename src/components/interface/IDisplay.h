#ifndef IDISPLAY_H
#define IDISPLAY_H

#include "IID.h"

typedef uint8_t display_status_t;

typedef uint8_t display_accu_t;
typedef uint8_t display_accu_temp_t;

typedef uint8_t display_speed_t;

typedef uint8_t display_controller_temp_t;
typedef uint8_t display_motor_temp_t;

typedef uint8_t display_state_t;
enum display_state_type_t : display_state_t {
    DISPLAY_HV_OFF = 0x0,
    DISPLAY_BOOTUP,
    DISPLAY_CALIBRARTION,
    DISPLAY_ALMOST_READY_TO_DRIVE,
    DISPLAY_READY_TO_DRIVE,
    DISPLAY_ERROR
};

class IDisplay : public IID {
    virtual void setStatus(display_status_t) = 0;
    virtual display_status_t getStatus() = 0;

    virtual void setAccu(display_accu_t accu) = 0;
    virtual display_accu_t getAccu() = 0;
    virtual void setAccuTemp(display_accu_temp_t temp) = 0;
    virtual display_accu_temp_t getAccuTemp() = 0;

    virtual void setSpeed(display_speed_t speed) = 0;
    virtual display_speed_t getSpeed() = 0;

    virtual void setControllerTemp(display_controller_temp_t temp) = 0;
    virtual display_controller_temp_t getControllerTemp() = 0;
    virtual void setMotorTemp(display_motor_temp_t temp) = 0;
    virtual display_motor_temp_t getMotorTemp = 0;

    virtual void setState(display_state_type_t state) = 0;
    virtual display_state_type_t getState() = 0;
};

#endif // IDISPLAY_H