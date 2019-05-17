#ifndef IMOTORCONTROLLER_H
#define IMOTORCONTROLLER_H

#include "IComponent.h"
#include "ILogable.h"

#define MOTOR_CONTROLLER_SD_LOG_COUNT 5
enum sd_log_id_motor_controller_t : sd_log_id_t {
    SD_LOG_ID_MOTOR_CONTROLLER_SPEED = 0,
    SD_LOG_ID_MOTOR_CONTROLLER_CURRENT = 1,
    SD_LOG_ID_MOTOR_CONTROLLER_MOTOR_TEMP = 2,
    SD_LOG_ID_MOTOR_CONTROLLER_SERVO_TEMP = 3,
    SD_LOG_ID_MOTOR_CONTROLLER_AIR_TEMP = 4
};

enum motor_controller_error_type_t : status_t {
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

        // Enable setter
        virtual void setRFE(motor_controller_rfe_enable_t state) = 0;
        virtual void setRUN(motor_controller_run_enable_t state) = 0;

        // Method to send interval times
        virtual void beginCommunication() = 0;

        // Logable
        virtual sd_log_id_t getLogValueCount() {
            return MOTOR_CONTROLLER_SD_LOG_COUNT;
        }

        virtual void getLogValue(string &logValue, sd_log_id_t logId) {
            if (logId >= MOTOR_CONTROLLER_SD_LOG_COUNT) return;

            char buffer[7];
            switch (logId) {
                case SD_LOG_ID_MOTOR_CONTROLLER_SPEED:
                    sprintf(buffer, "%1.5f", getSpeed());
                    logValue = buffer;
                    break;
                
                case SD_LOG_ID_MOTOR_CONTROLLER_CURRENT:
                    sprintf(buffer, "%1.5f", getCurrent());
                    logValue = buffer;
                    break;
                
                case SD_LOG_ID_MOTOR_CONTROLLER_MOTOR_TEMP:
                    logValue = to_string(getMotorTemp());
                    break;
                
                case SD_LOG_ID_MOTOR_CONTROLLER_SERVO_TEMP:
                    logValue = to_string(getServoTemp());
                    break;
                
                case SD_LOG_ID_MOTOR_CONTROLLER_AIR_TEMP:
                    logValue = to_string(getAirTemp());
                    break;
            }
        }
};

#endif // IMOTORCONTROLLER_H