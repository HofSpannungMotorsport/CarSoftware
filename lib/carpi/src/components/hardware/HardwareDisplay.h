#ifndef HARDWARE_DISPLAY_H
#define HARDWARE_DISPLAY_H

#include "../interface/IDisplay.h"

class HardwareDisplay : public IDisplay
{
public:
    HardwareDisplay()
    {
        setComponentType(COMPONENT_DISPLAY);
        setObjectType(OBJECT_HARDWARE);
    }

    HardwareDisplay(id_sub_component_t componentSubId) : HardwareDisplay()
    {
        setComponentSubId(componentSubId);
    }

    virtual void setStatus(display_status_t status)
    {
        pcSerial.printf("[HardwareDisplay]@parseMessage: status: 0x%x", status);
    }

    virtual void setSpeed(float speed)
    {
        pcSerial.printf("[HardwareDisplay]@parseMessage: speed: 0x%x", speed);
    }

    virtual void setBatteryVoltage(float batteryVoltage)
    {
        pcSerial.printf("[HardwareDisplay]@parseMessage: batteryVoltage: 0x%x", batteryVoltage);
    }

    virtual void setCellVoltage(float cellVoltage)
    {
        pcSerial.printf("[HardwareDisplay]@parseMessage: cellVoltage: 0x%x", cellVoltage);
    }

    virtual void setCellTemperature(float cellTemperature)
    {
        pcSerial.printf("[HardwareDisplay]@parseMessage: cellVoltage: 0x%x", cellTemperature);
    }

    virtual display_status_t getStatus()
    {
        return _status;
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
            if (subMessage.length != 7) // not a valid message
                result = MESSAGE_PARSE_ERROR;

            this->setStatus(subMessage.data[0]);

            uint16_t newValue = subMessage.data[1];

            this->setSpeed(newValue);

            /*

                 newValue16 = subMessage.data[3] | (subMessage.data[4] << 8);
                 newValue = (float)newValue16 / 65535.0;

                this->setBatteryVoltage(newValue);


                 newValue16 = subMessage.data[5] | (subMessage.data[6] << 8);
                 newValue = (float)newValue16 / 65535.0;

                this->setCellVoltage(newValue);

                float batteryTemperatureFloat = this->getBatteryTemperature();
                uint16_t batteryTemperature = ((float)batteryTemperatureFloat * 10);
                subMessage.data[5] = batteryTemperature & 0xFF;
                subMessage.data[6] = (batteryTemperature >> 8) & 0xFF;

                subMessage.data[9] = this->getIMD();
                subMessage.data[10] = this->getBMS();
                subMessage.data[11] = this->getShutdown();

            */

            return result;
        }
    }

private:
    display_status_t _status = 0;
    float _batteryVoltage = 403.2f;
    float _cellVoltage = 3.5f;
    float _batteryTemperature = 40.0f;
    float _speed = 23.4f;
    bool _imd = true;
    bool _bms = true;
    bool _shutdown = true;
};

#endif // HARDWARE_DISPLAY_H