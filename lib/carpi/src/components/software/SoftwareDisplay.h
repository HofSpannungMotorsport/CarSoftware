#ifndef SOFTWAREDISPLAY_H
#define SOFTWAREDISPLAY_H

#include "../interface/IDisplay.h"

class SoftwareDisplay : public IDisplay
{
public:
    SoftwareDisplay()
    {
        setComponentType(COMPONENT_DISPLAY);
        setObjectType(OBJECT_SOFTWARE);
    }

    SoftwareDisplay(id_sub_component_t componentSubId) : SoftwareDisplay()
    {
        setComponentSubId(componentSubId);
    }

    void setStatus(display_status_t status)
    {
        _status = status;
    }

    display_status_t getStatus()
    {
        return _status;
    }

    void setSpeed(float speed)
    {
        _speed = speed;
    }

    void setBatteryVoltage(float batteryVoltage)
    {
        _batteryVoltage = batteryVoltage;
    }

    void setCurrent(uint16_t current)
    {
        _current = current;
    }

    void setShutdownError(uint8_t shutdown)
    {
        _shutdown = shutdown;
    }

    void setPowermode(uint8_t powermode)
    {
        _powermode = powermode;
    }

    void setGas(float gas)
    {
        _gas = gas;
    }

    void setBrake(float brake)
    {
        _brake = brake;
    }

    void setState(uint8_t state)
    {
        _state = state;
    }

    void setMotorTemperature(float motorTemperature)
    {
        _motorTemperature = motorTemperature;
    }

    void setAirTemperature(float airTemperature)
    {
        _airTemperature = airTemperature;
    }

    float getBatteryVoltage()
    {
        return _batteryVoltage;
    }

    float getSpeed()
    {
        return _speed;
    }

    message_build_result_t buildMessage(CarMessage &carMessage)
    {
        car_sub_message_t subMessage;

        subMessage.length = 7;

        uint8_t msgId = 0;
        subMessage.data[0] = msgId;

        subMessage.data[1] = (uint8_t)_status;

        subMessage.data[2] = (uint8_t)_shutdown;

        uint16_t speed = (uint16_t)(_speed * 100);
        subMessage.data[3] = (uint8_t)(speed >> 8) & 0xFF;
        subMessage.data[4] = (uint8_t)(speed & 0xFF);

        subMessage.data[5] = (uint8_t)(_current >> 8) & 0xFF;
        subMessage.data[6] = (uint8_t)(_current & 0xFF);

        carMessage.addSubMessage(subMessage);

        msgId = 1;
        subMessage.data[0] = msgId;

        uint16_t motorTemp = ((float)_motorTemperature * 100);
        subMessage.data[1] = (uint8_t)(motorTemp >> 8) & 0xFF;
        subMessage.data[2] = (uint8_t)(motorTemp & 0xFF);

        uint16_t airTemp = ((float)_airTemperature * 100);
        subMessage.data[3] = (uint8_t)(airTemp >> 8) & 0xFF;
        subMessage.data[4] = (uint8_t)(airTemp & 0xFF);

        uint16_t dcVoltage = ((float)_batteryVoltage * 100);
        subMessage.data[5] = (uint8_t)(dcVoltage >> 8) & 0xFF;
        subMessage.data[6] = (uint8_t)(dcVoltage & 0xFF);

        carMessage.addSubMessage(subMessage);

        subMessage.length = 5;
        msgId = 2;
        subMessage.data[0] = msgId;
        subMessage.data[1] = (uint8_t)(_current >> 8) & 0xFF;
        subMessage.data[2] = (uint8_t)(_current & 0xFF);
        subMessage.data[3] = _state;
        subMessage.data[4] = _powermode;

        carMessage.addSubMessage(subMessage);

        msgId = 3;
        subMessage.data[0] = msgId;

        uint16_t pedalValue = ((float)_gas * 65535);
        subMessage.data[1] = pedalValue & 0xFF;
        subMessage.data[2] = (pedalValue >> 8) & 0xFF;

        pedalValue = ((float)_brake * 65535);
        subMessage.data[3] = pedalValue & 0xFF;
        subMessage.data[4] = (pedalValue >> 8) & 0xFF;

        carMessage.addSubMessage(subMessage);

        return MESSAGE_BUILD_OK;
    }

    message_parse_result_t parseMessage(CarMessage &carMessage)
    {
        message_parse_result_t result = MESSAGE_PARSE_OK;
        for (car_sub_message_t &subMessage : carMessage.subMessages)
        {
            if (subMessage.length != 1) // not a valid message
                result = MESSAGE_PARSE_ERROR;

            // this->setStatus(subMessage.data[0]);
        }

        return result;
    }

private:
    display_status_t _status = 0;
    float _batteryVoltage;
    float _motorTemperature;
    float _airTemperature;
    float _speed;
    uint16_t _current;
    float _gas;
    float _brake;
    uint8_t _shutdown;
    uint8_t _powermode;
    uint8_t _state;
};
#endif