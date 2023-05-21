#ifndef HARDWARE_DISPLAY_H
#define HARDWARE_DISPLAY_H

#define LVGL_TICK 10    // Time tick value for lvgl in ms (1-10msa)
#define TICKER_TIME 1ms // modified to miliseconds

Ticker ticker; // Initialize your system tick

#include "../interface/IDisplay.h"
#include "../lvgl.h"

#include "../lib/hal_stm_lvgl/touchpad/touchpad.h"
#include "../lib/hal_stm_lvgl/tft/tft.h"

#include "SharedMutex.h"

#include "ui.h"

class HardwareDisplay : public IDisplay
{
public:
    HardwareDisplay(SharedMutex &mutex) : _mutex(mutex)
    {
        setComponentType(COMPONENT_DISPLAY);
        setObjectType(OBJECT_HARDWARE);

        setup();
    }

    HardwareDisplay(SharedMutex &mutex, id_sub_component_t componentSubId) : HardwareDisplay(mutex)
    {
        setComponentSubId(componentSubId);
    }

    void lv_task()
    {
        while (true)
        {
            _mutex.lock();
            lv_task_handler();
            _mutex.unlock();

            ThisThread::sleep_for(10);
        }
    }

    void setBatteryVoltage(float value)
    {
        _batteryVoltage = value;
        lv_label_set_text_fmt(ui_Label2, "%dV", (uint16_t)_batteryVoltage);
        // map value 288-403 to 0-100
        // uint8_t batteryPercentage = (uint8_t)((_batteryVoltage - 288.0) / (403.0 - 288.0) * 100.0);
        uint8_t batteryPercentage = (uint8_t)((_batteryVoltage - 4.0) / (60.0 - 4.0) * 100.0);
        lv_bar_set_value(ui_Bar1, batteryPercentage, LV_ANIM_OFF);
        lv_bar_set_value(ui_Bar2, batteryPercentage, LV_ANIM_OFF);
        lv_label_set_text_fmt(ui_Label1, "%d%%", batteryPercentage);
    }
    void setCellVoltage(float value)
    {
        _cellVoltage = value;
    }
    void setCellTemperatureMin(float value)
    {
    }
    void setCellTemperatureMax(float value)
    {
    }
    void setSpeed(float value)
    {
        _speed = value;
        lv_label_set_text_fmt(ui_Label6, "%d", (uint8_t)_speed);
    }
    void setAirTemperature(float value)
    {
        _airTemperature = value;
        lv_label_set_text_fmt(ui_Label4, "%d°C", (uint8_t)_airTemperature);
    }

    void setMotorTemperature(float value)
    {
        _motorTemperature = value;
        lv_label_set_text_fmt(ui_Label3, "%d°C", (uint8_t)_motorTemperature);
    }
    void setCurrent(float value)
    {
        _current = value;
    }
    void setPower(uint32_t value)
    {
        _power = value;
    }
    void setShutdownError(uint8_t value)
    {
        _shutdownError = value;
    }
    void setPowermode(uint8_t value)
    {
        _powermode = value;
    }
    void setGas(float value)
    {
        _gas = value;
        lv_bar_set_value(ui_Bar5, (uint8_t)(_gas * 100), LV_ANIM_OFF);
    }
    void setBrake(float value)
    {
        _brake = value;
        lv_bar_set_value(ui_Bar6, (uint8_t)(_brake * 100), LV_ANIM_OFF);
    }

    void setIMD(bool value)
    {
        _imd = value;
    }
    void setBMS(bool value)
    {
        _bms = value;
    }

    void setStatus(display_status_t status)
    {
        _status = status;
    }

    display_status_t getStatus()
    {
        return _status;
    }

    message_build_result_t buildMessage(CarMessage &carMessage)
    {
        return MESSAGE_BUILD_OK;
    }

    message_parse_result_t parseMessage(CarMessage &carMessage)
    {
        message_parse_result_t result = MESSAGE_PARSE_OK;
        for (car_sub_message_t &subMessage : carMessage.subMessages)
        {
            if (subMessage.length < 5 || subMessage.length > 7) // not a valid message
                result = MESSAGE_PARSE_ERROR;

            uint8_t msgId;
            msgId = subMessage.data[0];
            if (msgId == 0)
            {

                // subMessage.data[1] = (uint8_t)_status;

                setShutdownError(subMessage.data[2]);

                uint16_t speed = (uint16_t)subMessage.data[3] | (subMessage.data[4] << 8);
                setSpeed((float)speed / 100.0);

                uint16_t current = (uint16_t)subMessage.data[5] | (subMessage.data[6] << 8);
                setCurrent((float)current / 100.0);
            }
            else if (msgId == 1)
            {

                uint16_t motorTemp = (uint16_t)subMessage.data[1] << 8 | subMessage.data[2];
                setMotorTemperature((float)motorTemp / 100.0);

                uint16_t airTemp = (uint16_t)subMessage.data[3] << 8 | subMessage.data[4];
                setAirTemperature((float)airTemp / 100.0);

                uint16_t dcVoltage = (uint16_t)subMessage.data[5] << 8 | subMessage.data[6];
                setBatteryVoltage((float)dcVoltage / 100.0);
            }
            else if (msgId == 2)
            {
                uint32_t power = subMessage.data[1] << 24 | subMessage.data[2] << 16 | subMessage.data[3] << 8 | subMessage.data[4];
                setPower(power);

                setPowermode(subMessage.data[5]);
            }
            else if (msgId == 3)
            {
                uint16_t gas = (uint16_t)subMessage.data[1] | (subMessage.data[2] << 8);
                setGas((float)gas / 65535.0);

                uint16_t brake = (uint16_t)subMessage.data[3] | (subMessage.data[4] << 8);
                setBrake((float)brake / 65535.0);
            }
            else
                result = MESSAGE_PARSE_ERROR;
        }

        return result;
    }

    static void lv_ticker_func()
    {
        lv_tick_inc(LVGL_TICK);
    }

    void setup()
    {
        lv_init();
        tft_init();      // Initialize diplay
        touchpad_init(); // Initialize touchpad

        ticker.attach(callback(&lv_ticker_func), TICKER_TIME); // Attach callback to ticker
        ui_init();
    }

private:
    SharedMutex &_mutex;
    display_status_t _status = 0;
    float _batteryVoltage = 403.2f;
    float _cellVoltage = 3.5f;
    float _batteryTemperature = 40.0f;
    float _speed = 23.4f;
    bool _imd = true;
    bool _bms = true;
    float _motorTemperature = 40.0f;
    float _airTemperature = 40.0f;
    float _current = 0.0f;
    uint32_t _power = 0;
    uint8_t _shutdownError = 0;
    uint8_t _powermode = 0;
    float _gas = 0.0f;
    float _brake = 0.0f;
};

#endif // HARDWARE_DISPLAY_H