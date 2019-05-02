#ifndef IALIVE_H
#define IALIVE_H

#include "communication/SelfSyncable.h"

#define STD_ALIVE_MESSAGE_TIMEOUT 0.01 // s
#define STD_ALIVE_SIGNAL_REFRESH_RATE 0.1 // s between two alive signals

typedef uint8_t alive_status_t;
enum alive_error_type_t : alive_status_t {
    ALIVE_ERROR_DEAD = 0x1
};

enum alive_message_command_t : uint8_t {
    ALIVE_MESSAGE_SEND_ALIVE = 0x0
};

class IAlive : public SelfSyncable {
    public:
        virtual void setAlive(bool alive) = 0;
        virtual bool getAlive() = 0;

        virtual alive_status_t getStatus() = 0;

        virtual void receive(CarMessage &carMessage) {
            for(car_sub_message_t &subMessage : carMessage.subMessages) {
                if (subMessage.length == 2) {
                    switch (subMessage.data[0]) {
                        case ALIVE_MESSAGE_SEND_ALIVE:
                            if (subMessage.data[1] == 0x1)
                                setAlive(true);
                            else
                                setAlive(false);
                            break;
                    }
                }
            }
        }
};

#endif // IALIVE_H