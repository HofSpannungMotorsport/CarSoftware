#ifndef IPUMP_H
#define IPUMP_H

#include "IComponent.h"

#define PUMP_SD_LOG_ID_COUNT 1
enum sd_log_id_pump_t : sd_log_id_t {
    SD_LOG_ID_PUMP_SPEED = 1,
};

typedef float pump_speed_t;
typedef int pump_enable_t;

class IPump : public IComponent {
    public:
        IPump() {
            setComponentType(COMPONENT_COOLING);
        }

        virtual void setSpeed(pump_speed_t speed) = 0;
        virtual pump_speed_t getSpeed() = 0;

        virtual void setEnable(pump_enable_t enable) = 0;
        virtual pump_enable_t getEnable() = 0;

        virtual sd_log_id_t getLogValueCount() {
            return PUMP_SD_LOG_ID_COUNT;
        }

        virtual void getLogValue(string &logValue, sd_log_id_t logId) {
            if (logId != 0) return;

            char buffer[7];
            sprintf(buffer, "%1.5f", getSpeed());
            logValue = buffer;
        }
};

#endif // IPUMP_H