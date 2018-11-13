#ifndef RPM_SENSOR_MESSAGE_HANDLER_H
#define RPM_SENSOR_MESSAGE_HANDLER_H

#include "mbed.h"
#include "IMessageHandler.h"
#include "../components/interface/IRpmSensor.h"

#define CAN_RPM_SENSOR_PAYLOAD_LENGTH 5

class RpmSensorMessageHandler : public IMessageHandler<CANMessage> {
    public:
        virtual msg_build_result_t buildMessage(void *objV, CANMessage &msg) {
            IRpmSensor *obj = (IRpmSensor*)objV;
            msg.len = CAN_RPM_SENSOR_PAYLOAD_LENGTH;

            msg.data[0] = obj->getStatus();

            rpm_sensor_frequency_t frequency = obj->getFrequency();
            uint32_t frequencyBinary = *((uint32_t*)&frequency);

            // Slice data in 4 Byte -> 32bit float
            for (uint8_t i = 1; i < CAN_RPM_SENSOR_PAYLOAD_LENGTH; i++) {
                msg.data[i] = (uint8_t)((frequencyBinary >> ((i - 1) * 8)) & 0xFF);
            }

            return MSG_BUILD_OK;
        }

        virtual msg_parse_result_t parseMessage(void *objV, CANMessage &msg) {
            IRpmSensor *obj = (IRpmSensor*)objV;

            if(msg.len != CAN_RPM_SENSOR_PAYLOAD_LENGTH) // not a valid message
                return MSG_PARSE_ERROR;

            obj->setStatus(msg.data[0]);

            uint32_t frequencyBinary = 0;

            for (uint8_t i = 1; i < CAN_RPM_SENSOR_PAYLOAD_LENGTH; i++) {
                frequencyBinary |= (((uint32_t)msg.data[i]) << ((i - 1) * 8));
            }

            rpm_sensor_frequency_t frequency = *((rpm_sensor_frequency_t*)&frequencyBinary);
            obj->setFrequency(frequency);

            return MSG_PARSE_OK;
        }
};

#endif // RPM_SENSOR_MESSAGE_HANDLER_H