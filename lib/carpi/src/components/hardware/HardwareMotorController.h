#ifndef HARDWAREMOTORCONTROLLER_H
#define HARDWAREMOTORCONTROLLER_H

#include "BamocarD3.h"
#include "../interface/IMotorController.h"

#define STD_SPEED_REFRESH_TIME 240 // Hz
#define STD_CURRENT_REFRESH_TIME 30 // Hz
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

    int16_t getMotorTemp()
    {
        return _bamocar.getMotorTemp();
    }

    float getMotorTempAge()
    {
        return _bamocar.getMotorTempAge();
    }

    int16_t getServoTemp()
    {
        return _bamocar.getServoTemp();
    }

    float getServoTempAge()
    {
        return _bamocar.getServoTempAge();
    }

    int16_t getAirTemp()
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
        car_sub_message_t subMessage;

        subMessage.length = 7;
        /*

                subMessage.data[0] = 4;
                subMessage.data[1] = 334;
                subMessage.data[2] = 99;
                subMessage.data[3] = 45;
                pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject [0]: %x\t[1]: %x\t[2]: %x\t[3]: %x\n", subMessage.data[0], subMessage.data[1], subMessage.data[2], subMessage.data[3]);
        */
        uint8_t msgId = 0;
        subMessage.data[0] = msgId;

        uint8_t status = (uint8_t)this->getStatus();
        subMessage.data[1] = status;

        uint8_t state = (uint8_t)this->getState();
        subMessage.data[2] = state;

        float speedFloat = this->getSpeed();
        // float speedFloat = 80.37f;
        uint16_t speed = (uint16_t)(speedFloat * 100);
        subMessage.data[3] = (uint8_t)(speed >> 8) & 0xFF;
        subMessage.data[4] = (uint8_t)(speed & 0xFF);

        float currentFloat = this->getCurrent();
        // float currentFloat = 180.7f;
        uint16_t current = ((float)currentFloat * 100);
        subMessage.data[5] = (uint8_t)(current >> 8) & 0xFF;
        subMessage.data[6] = (uint8_t)(current & 0xFF);

        carMessage.addSubMessage(subMessage);

        /*
                pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject (uint8_t)speed: %i\t", status);
                pcSerial.printf("msg.data[2]: 0x%x\n", subMessage.data[0]);
                pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject (uint8_t)speed: %i\t", state);
                pcSerial.printf("msg.data[2]: 0x%x\n", subMessage.data[1]);
                pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject (uint8_t)speed: %i\t", speed);
                pcSerial.printf("msg.data[2]: 0x%x\n", subMessage.data[2]);
                pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject (uint8_t)motorTemp: %i\t", current);
                pcSerial.printf("msg.data[3]: 0x%x\n", subMessage.data[3]);
                pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject (uint8_t)motorTemp: %i\t", motorTemp);
                pcSerial.printf("msg.data[4]: 0x%x\n", subMessage.data[4]);
                pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject (uint8_t)servoTemp: %i\t", airTemp);
                pcSerial.printf("msg.data[5]: 0x%x\n", subMessage.data[5]);
                pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject (uint8_t)dcVoltage: %i\t", dcVoltage);
                pcSerial.printf("msg.data[6]: 0x%x\n", subMessage.data[6]);
        */
        /*

                subMessage.data[0] = 4;
                subMessage.data[1] = 334;
                subMessage.data[2] = 99;
                subMessage.data[3] = 45;
                pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject [0]: %x\t[1]: %x\t[2]: %x\t[3]: %x\n", subMessage.data[0], subMessage.data[1], subMessage.data[2], subMessage.data[3]);
        */

        subMessage.length = 5;

        msgId = 1;
        subMessage.data[0] = msgId;

        int8_t motorTemp = this->getMotorTemp();
        subMessage.data[1] = motorTemp;

        int8_t airTemp = this->getAirTemp();
        subMessage.data[2] = airTemp;

        float dcVoltageFloat = this->getDcVoltage();
        // float dcVoltageFloat = 403.21f;
        uint16_t dcVoltage = (uint16_t)(dcVoltageFloat * 100);
        subMessage.data[3] = (uint8_t)(dcVoltage >> 8) & 0xFF;
        subMessage.data[4] = (uint8_t)(dcVoltage & 0xFF);
        /*
                pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject (uint8_t)speed: %i\t", status);
                pcSerial.printf("msg.data[2]: 0x%x\n", subMessage.data[0]);
                pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject (uint8_t)speed: %i\t", state);
                pcSerial.printf("msg.data[2]: 0x%x\n", subMessage.data[1]);
                pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject (uint8_t)speed: %i\t", speed);
                pcSerial.printf("msg.data[2]: 0x%x\n", subMessage.data[2]);
                pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject (uint8_t)motorTemp: %i\t", current);
                pcSerial.printf("msg.data[3]: 0x%x\n", subMessage.data[3]);
                pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject (uint8_t)motorTemp: %i\t", motorTemp);
                pcSerial.printf("msg.data[4]: 0x%x\n", subMessage.data[4]);
                pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject (uint8_t)servoTemp: %i\t", airTemp);
                pcSerial.printf("msg.data[5]: 0x%x\n", subMessage.data[5]);
                pcSerial.printf("[HardwarePedal]@buildMessage: HardwareObject (uint8_t)dcVoltage: %i\t", dcVoltage);
                pcSerial.printf("msg.data[6]: 0x%x\n", subMessage.data[6]);
          */
        carMessage.addSubMessage(subMessage);
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

    motor_controller_status_t _status = 0;
};

#endif // HARDWAREMOTORCONTROLLER_H