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

    void setCurrent(float current)
    {
        _current = current;
    }

    void setShutdownError(uint8_t shutdown)
    {
        _shutdown = shutdown;
    }

    void setPower(uint32_t power)
    {
        _power = power;
    }
    void setPowermode(uint16_t powermode)
    {
        _powermode = powermode;
    }

    void setCellVoltage(float cellVoltage)
    {
        _cellVoltage = cellVoltage;
    }

    void setMotorTemperature(float motorTemperature)
    {
        _motorTemperature = motorTemperature;
    }

    void setAirTemperature(float airTemperature)
    {
        _airTemperature = airTemperature;
    }

    void setCellTemperatureMax(float cellTemperatureMax)
    {
        _cellTemperatureMax = cellTemperatureMax;
    }
    
    void setCellTemperatureMin(float cellTemperatureMin)
    {
        _cellTemperatureMin = cellTemperatureMin;
    }

    float getBatteryVoltage()
    {
        return _batteryVoltage;
    }

    float getCellVoltage()
    {
        return _cellVoltage;
    }

    float getCellTemperatureMax()
    {
        return _cellTemperatureMax;
    }

    float getCellTemperature()
    {
        return _cellTemperatureMin;
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

        subMessage.data[1] = (uint8_t) _status;

        subMessage.data[2] = (uint8_t) _shutdown;

        // float speedFloat = 80.37f;
        uint16_t speed = (uint16_t)(_speed * 100);
        subMessage.data[3] = (uint8_t)(speed >> 8) & 0xFF;
        subMessage.data[4] = (uint8_t)(speed & 0xFF);

        // float currentFloat = 180.7f;
        uint16_t current = ((float)_current * 100);
        subMessage.data[5] = (uint8_t)(current >> 8) & 0xFF;
        subMessage.data[6] = (uint8_t)(current & 0xFF);


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

        subMessage.data[1]  = (uint8_t)((_power >> 24) & 0xFF);
        subMessage.data[2]  = (uint8_t)((_power >> 16) & 0xFF);
        subMessage.data[3]  = (uint8_t)((_power >> 8) & 0xFF);
        subMessage.data[4]  = (uint8_t)(_power & 0xFF);

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

            this->setStatus(subMessage.data[0]);
        }

        return result;
    }

private:
    display_status_t _status = 0;
    float _batteryVoltage;
    float _cellVoltage = 3.5f;
    float _cellTemperatureMax = 50.0f;
    float _cellTemperatureMin = 40.0f;
    float _motorTemperature;
    float _airTemperature;
    float _speed;
    float _current;
    uint32_t _power;
    uint8_t _shutdown;
    uint8_t _powermode;
};
#endif