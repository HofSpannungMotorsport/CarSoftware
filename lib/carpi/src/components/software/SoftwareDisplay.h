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

    virtual void setStatus(display_status_t status)
    {
        _status = status;
    }

    virtual display_status_t getStatus()
    {
        return _status;
    }

    virtual void setSpeed(float speed)
    {
        _speed = speed;
    }

    virtual void setBatteryVoltage(float batteryVoltage)
    {
        _batteryVoltage = batteryVoltage;
    }

    virtual void setCellVoltage(float cellVoltage)
    {
        _cellVoltage = cellVoltage;
    }

    virtual void setCellTemperature(float cellTemperature)
    {
        _cellTemperature = cellTemperature;
    }

    virtual float getBatteryVoltage()
    {
        return _batteryVoltage;
    }

    virtual float getCellVoltage()
    {
        return _cellVoltage;
    }

    virtual float getCellyTemperature()
    {
        return _cellTemperature;
    }

    virtual float getSpeed()
    {
        return _speed;
    }
    virtual message_build_result_t buildMessage(CarMessage &carMessage)
    {
        car_sub_message_t subMessage;

        subMessage.length = 7;

        subMessage.data[0] = this->getStatus();

        uint8_t speed = (int)this->getSpeed();
        subMessage.data[1] = speed;

        /*
        subMessage.data[2] = (speed >> 8) & 0xFF;

        float batteryVoltageFloat = this->getBatteryVoltage();
        float batteryPercentFloat = ((float)(batteryVoltageFloat - 240.0f) * (255.0f / (403.0f - 240.0f)));

        subMessage.data[3] = batteryVoltage & 0xFF;
        subMessage.data[4] = (batteryVoltage >> 8) & 0xFF;

        uint8_t batteryPercent = ((uint8_t)batteryPercentFloat);
        subMessage.data[3] = batteryPercent;
        subMessage.data[4] = batteryPercent;

        float cellVoltageFloat = this->getCellVoltage();
        uint16_t cellVoltage = ((float)cellVoltageFloat * 1000);
        subMessage.data[5] = cellVoltage & 0xFF;
        subMessage.data[6] = (cellVoltage >> 8) & 0xFF;

        pcSerial.printf("3: %x", subMessage.data[3]);
        pcSerial.printf("4: %x", subMessage.data[4]);
        pcSerial.printf("5: %x", subMessage.data[5]);
        pcSerial.printf("6: %x", subMessage.data[6]);

        float _cellTemperatureFloat = this->getCellTemperature();
        uint16_t _cellTemperature = ((float)cellTemperatureFloat * 10);
        subMessage.data[5] = cellTemperature & 0xFF;
        subMessage.data[6] = (cellTemperature >> 8) & 0xFF;

        subMessage.data[9] = this->getIMD();
        subMessage.data[10] = this->getBMS();
        subMessage.data[11] = this->getShutdown();
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

            this->setStatus(subMessage.data[0]);
        }

        return result;
    }

private:
    display_status_t _status = 0;
    float _batteryVoltage = 403.2f;
    float _cellVoltage = 3.5f;
    float _cellTemperature = 40.0f;
    float _speed = 23.4f;
};
#endif