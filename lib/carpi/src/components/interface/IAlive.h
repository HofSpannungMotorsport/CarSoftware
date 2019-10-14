#ifndef IALIVE_H
#define IALIVE_H

#include "communication/SelfSyncable.h"

#define ALIVE_LOG_VALUE_COUNT 1
enum sd_log_id_alive_t : sd_log_id_t {
    SD_LOG_ID_ALIVE = 0
};

enum alive_error_type_t : status_t {
    ALIVE_ERROR_DEAD = 0x1
};

enum alive_message_command_t : uint8_t {
    ALIVE_MESSAGE_SEND_ALIVE = 0x0
};

class IAlive : public SelfSyncable {
    public:
        virtual void setAlive(bool alive) = 0;
        virtual bool getAlive() = 0;

        virtual sd_log_id_t getLogValueCount() {
            return ALIVE_LOG_VALUE_COUNT;
        }

        virtual void getLogValue(string &logValue, sd_log_id_t logId) {
            if (logId != 0) return;

            if (getAlive()) {
                logValue = "t";
            } else {
                logValue = "f";
            }
        }
};

#endif // IALIVE_H