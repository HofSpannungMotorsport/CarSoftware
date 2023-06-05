#ifndef HARDWAREMOTORCONTROLLER_H
#define HARDWAREMOTORCONTROLLER_H

#include "BamocarD3.h"
#include "../interface/IMotorController.h"

#define STD_SPEED_REFRESH_TIME 240          // Hz
#define STD_CURRENT_REFRESH_TIME 30         // Hz
#define STD_CURRENT_DEVICE_REFRESH_TIME 240 // Hz
#define STD_MOTOR_TEMP_REFRESH_TIME 12      // Hz
#define STD_CONTROLLER_TEMP_REFRESH_TIME 12 // Hz
#define STD_AIR_TEMP_REFRESH_TIME 6         // Hz
#define STD_MOTOR_VOLTAGE_REFRESH_TIME 240  // Hz

class HardwareMotorController : public IMotorController
{
public:
    HardwareMotorController(PinName canRD, PinName canTD, PinName RFE, PinName RUN, model_type_t modelType, bool invertTorque)
        : _bamocar(canRD, canTD, invertTorque, modelType), _rfe(RFE), _run(RUN)
    {
        _rfe = 0;
        _run = 0;

        setComponentType(COMPONENT_MOTOR);
        setObjectType(OBJECT_HARDWARE);
    }

    HardwareMotorController(PinName canRD, PinName canTD, PinName RFE, PinName RUN, model_type_t modelType, id_sub_component_t componentSubId, bool invertTorque)
        : HardwareMotorController(canRD, canTD, RFE, RUN, modelType, invertTorque)
    {
        setComponentSubId(componentSubId);
    }

    void beginCommunication()
    {
        _bamocar.requestSpeed(1000 / (float)STD_SPEED_REFRESH_TIME);
        _bamocar.requestCurrent(1000 / (float)STD_CURRENT_REFRESH_TIME);
        _bamocar.requestTemp(1000 / (float)STD_MOTOR_TEMP_REFRESH_TIME);
        _bamocar.requestDcVoltage(1000 / (float)STD_MOTOR_VOLTAGE_REFRESH_TIME);

        _bamocar.begin(true);
    }

    motor_controller_status_t getStatus()
    {
        return _status;
    }

    motor_controller_state_t getState()
    {
        // The Status of the Motor Controller
        // (not naming it Status because otherwise it conflicts with the Status naming of the other components,
        // so to have a consistant naming and to save errors in "Status", we use the naming State here)
        // [il]
        return 0;
    }

    void setTorque(float torque)
    {
        float setTorqueTo = torque;

        // Check if out of bounds (-1.0 - 1.0)
        if (torque > 1.0)
        {
            if (torque > 1.1)
            {
                // Given Value too far outside of our bounds (so not only float error)
                motor_controller_error_type_t badValError = MOTOR_CONTROLLER_BAD_VAR_GIVEN;
                _status = badValError;
                setTorqueTo = 0;
            }
            else
            {
                // Else, set to boundary
                setTorqueTo = 1.0;
            }
        }
        else if (torque < -1.0)
        {
            // A negative represents negative torque -> if only spinning into front direction, this will result in recuperation
            if (torque < (-1.1))
            {
                motor_controller_error_type_t badValError = MOTOR_CONTROLLER_BAD_VAR_GIVEN;
                _status = badValError;
                setTorqueTo = 0;
            }
            else
            {
                setTorqueTo = -1.0;
            }
        }

        _bamocar.setTorque(setTorqueTo);
    }

    float getSpeed()
    {
        return _bamocar.getSpeed();
    }

    float getSpeedAge()
    {
        return _bamocar.getSpeedAge();
    }

    float getCurrent()
    {
        return _bamocar.getCurrent();
    }

    uint16_t getSentCurrent()
    {
        return _sentCurrent;
    }

    void setSentCurrent(uint16_t current)
    {
        _sentCurrent = current;
    }

    float getMotorTemp()
    {
        return _bamocar.getMotorTemp();
    }

    float getMotorTempAge()
    {
        return _bamocar.getMotorTempAge();
    }

    float getServoTemp()
    {
        return _bamocar.getServoTemp();
    }

    float getServoTempAge()
    {
        return _bamocar.getServoTempAge();
    }

    float getAirTemp()
    {
        return _bamocar.getAirTemp();
    }

    float getAirTempAge()
    {
        return _bamocar.getAirTempAge();
    }

    float getDcVoltage()
    {
        return _bamocar.getDcVoltage();
    }

    float getDcVoltageAge()
    {
        return _bamocar.getDcVoltageAge();
    }

    void setRFE(motor_controller_rfe_enable_t state)
    {
        if (state == MOTOR_CONTROLLER_RFE_ENABLE)
        {
            _rfe = 1;
        }
        else
        {
            _rfe = 0;
        }
    }

    void setRUN(motor_controller_run_enable_t state)
    {
        if (state == MOTOR_CONTROLLER_RUN_ENABLE)
        {
            _run = 1;
        }
        else
        {
            _run = 0;
        }
    }

    virtual void setStatus(motor_controller_status_t status)
    {
        _status = status;
    }

    virtual message_build_result_t buildMessage(CarMessage &carMessage)
    {
        return MESSAGE_BUILD_OK;
    }

    virtual message_parse_result_t parseMessage(CarMessage &carMessage)
    {
        message_parse_result_t result = MESSAGE_PARSE_OK;
        for (car_sub_message_t &subMessage : carMessage.subMessages)
        {
            if (subMessage.length != 1) // not a valid message
                result = MESSAGE_PARSE_ERROR;
        }
        return result;
    }

protected:
    BamocarD3 _bamocar;
    DigitalOut _rfe;
    DigitalOut _run;
    uint32_t _sentCurrent = 0;

    motor_controller_status_t _status = 0;
};

#endif // HARDWAREMOTORCONTROLLER_H