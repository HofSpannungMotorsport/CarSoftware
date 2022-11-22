#ifndef DISPLAYSERVICE_H
#define DISPLAYSERVICE_H

#include "IService.h"
#include "SCar.h"
#include "../interface/IAccumulator.h"
#include "../interface/IDisplay.h"


class PDisplay : public IService {
    public:

        PDisplay(CANService &canService, IDisplay* display,
                IMotorController* motorController, IHvEnabled* hvEnabled)
            :_canService(canService){

            _display = display;
            _motorController = motorController;
            _hvEnabled = hvEnabled;
        }

        virtual void run() {
        }
       bool sendMessage() {
            component_exist_t componentExist =_registeredAddresses[id];


            if (componentExist.exists) {
                CarMessage m;
                component_details_t component = _components[id];

                m.setSenderId(senderId);
                m.setReceiverId(receiverId);
                message_build_result_t msgBuildResult = component.component->buildMessage(m);

                #ifdef CAN_DEBUG
                    if (msgBuildResult == MESSAGE_BUILD_OK) {
                        pcSerial.printf("[CANService]@sendMessage: Component 0x%x Message Build success\n", id);
                    } else {
                        pcSerial.printf("[CANService]@sendMessage: Component 0x%x Message Build error\n", id);
                    }
                #endif

                if (msgBuildResult == MESSAGE_BUILD_ERROR) return false;


                // Convert CarMessage to CANMessage and send all sub-messages
                int msgSendResult = 0;
                for (car_sub_message_t &subMessage : m.subMessages) {
                    msgSendResult = 0;

                    CANMessage canMessage = CANMessage();
                    canMessage.format = CANStandard;
                    canMessage.id = deviceId::getMessageHeader(m.getSenderId(), m.getReceiverId());
                    canMessage.len = subMessage.length + 1;
                    canMessage.data[0] = id;
                    
                    for (int i = 0; i < subMessage.length; i++) {
                        canMessage.data[i+1] = subMessage.data[i];
                    }

                    Timer timeout;
                    timeout.start();
                    while((msgSendResult != 1) && (timeout < CAN_TIMEOUT)) {
                        msgSendResult = _can.write(canMessage);
                        wait(0.00000124);
                    }
                }

                #ifdef CAN_DEBUG
                    pcSerial.printf("[CANService]@sendMessage: Message for Component 0x%x write result: %i (1 == Succes, 0 == Failed)\n", id, msgSendResult);
                #endif

                if (msgSendResult > 0) return true;
                else return false;
            } else { // else do nothing (component not registered before -> can't send messages for it == obvious)
                #ifdef CAN_DEBUG
                    pcSerial.printf("[CANService]@sendMessage: Can't send Message for Component 0x%x -> component not registered before (componentExist.exists == false)\n", id);
                #endif
            }

            return false;
        }

    protected:
        CANService &_canService;
        IAccumulator* _accumulator;
        IDisplay* _display;
        IMotorController* _motorController;
        IPedal* _gasPedal;
        IPedal* _brakePedal;
        IHvEnabled* _hvEnabled;
};

#endif // SPEEDSERVICE_H