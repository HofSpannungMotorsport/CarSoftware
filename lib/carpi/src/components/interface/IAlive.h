#ifndef IALIVE_H
#define IALIVE_H

#include "ICommunication.h"

typedef uint8_t alive_status_t;
enum alive_error_type_t : alive_status_t {
    ALIVE_ERROR_DEAD = 0x1
};

enum alive_message_command_t : uint8_t {
    ALIVE_MESSAGE_SEND_ALIVE = 0x0
};

class IAlive : public ICommunication {
    public:
        virtual void setAlive(bool alive) = 0;
        virtual bool getAlive() = 0;

        virtual alive_status_t getStatus() = 0;

        virtual message_build_result_t buildMessage(CarMessage &carMessage) {
            car_sub_message_t subMessage;
            subMessage.length = 2;
            subMessage.data[0] = ALIVE_MESSAGE_SEND_ALIVE;
            
            if (getAlive()) {
                subMessage.data[1] = 0x1;
            } else {
                subMessage.data[1] = 0x0;
            }

            carMessage.addSubMessage(subMessage);

            return MESSAGE_BUILD_OK;
        }

        virtual message_parse_result_t parseMessage(CarMessage &carMessage) {
            message_parse_result_t parseResult = MESSAGE_PARSE_OK;

            for(car_sub_message_t &subMessage : carMessage.subMessages) {
                if (subMessage.length == 2) {
                    switch (subMessage.data[0])
                    {
                        case ALIVE_MESSAGE_SEND_ALIVE:
                            if (subMessage.data[1] == 0x1)
                                setAlive(true);
                            else
                                setAlive(false);
                            break;
                    
                        default:
                            parseResult = MESSAGE_PARSE_ERROR;
                            break;
                    }
                } else {
                    parseResult = MESSAGE_PARSE_ERROR;
                }
            }

            return parseResult;
        }
};

#endif // IALIVE_H