#ifndef CCAN_H
#define CCAN_H

#include "HardConfig.h"

#ifndef STEROIDO_DEVICE_CAN
    #error "CAN is not supported or disabled on this device!"
#endif

#include "Sync/Sync.h"
#include "CarMessage.h"

class CCan : public IChannel {
    public:
        CCan() = delete;
        CCan(Sync &syncer, PinName rx, PinName tx, int frequency = STD_CCAN_FREQUENCY)
        : _can(rx, tx, frequency), _syncer(syncer) {
            #ifdef STEROIDO_CAN_INTERRUPT_ENABLED
                _can.attach(callback(this, &CCan::_canMessageReceive), CAN::RxIrq);
            #endif
        }

        /**
         * @brief Send a CarMessage over CAN
         * 
         * @param carMessage Reference to CarMessage to be sent
         * @return true If the Message was sent or enqueued for sending
         * @return false If the Message was not sent or not valid (?)
         */
        bool send(CarMessage &carMessage) {
            CANMessage canMessage;

            if (!_getCanMessage(carMessage, canMessage))
                return false;

            #if defined(CCAN_SENDING_DEBUG) && defined(MESSAGE_REPORT)
                printf("[CCan]@_send->_outQueue: Try to send canMessage with component ID 0x%x and message ID 0x%x\n", canMessage.data[0], canMessage.id);
            #endif

            // Now try to send the Message over CAN
            int canWriteResult = _can.write(canMessage);
            if (canWriteResult == 1) {
                // If sent successfully, return true

                #if defined(CCAN_SENDING_DEBUG) && defined(MESSAGE_REPORT)
                    printf("[CCan]@_send->_outQueue: Successfully sent canMessage with component ID 0x%x and message ID 0x%x\n", carMessage.getComponentId(), canMessage.id);
                #endif

                return true;
            } else {
                #if defined(CCAN_SENDING_DEBUG) && defined(MESSAGE_REPORT)
                    printf("[CCan]@_send->_outQueue: CAN Send Buffer full for Message with component ID 0x%x and message ID 0x%x\n", carMessage.getComponentId(), canMessage.id);
                #endif

                return false;
            }
        }

        /**
         * @brief Run the CAN Channel for detecting Errors and receiving Messages
         * 
         */
        void run() {
            #ifndef STEROIDO_CAN_INTERRUPT_ENABLED
                if (_can.available() > 0) {
                    _canMessageReceive();
                }
            #endif

            if (_can.tderror() > 0 || _can.rderror() > 0) {
                #if defined(MESSAGE_REPORT) && defined(USE_MBED)
                    printf("[CCan]@run: Detected CAN Error: td: %i\t rd: %i\n", _can.tderror(), _can.rderror());
                #endif

                _can.reset();
            }
        }
    
    private:
        CAN _can;
        Sync &_syncer;

        void _canMessageReceive() {
            CANMessage canMessage;

            while(_can.read(canMessage)) {
                CarMessage carMessage;
                if (_getCarMessage(canMessage, carMessage))
                    _syncer.receive(carMessage);
            }
        }

        /**
         * @brief Generate a CarMessage from a CANMessage
         * 
         * @param canMessage Reference to CANMessage
         * @param carMessage Reference to empty CarMessage
         * @return true If build was successfull
         * @return false If build failed
         */
        bool _getCarMessage(CANMessage &canMessage, CarMessage &carMessage) {
            bool returnVal = false;

            if (canMessage.format == CANStandard) {
                // Small Message has at least 2 data bytes for Data-related IDs
                if (canMessage.len >= 2) {
                    // ID
                    carMessage.setSenderId((id_device_t)(((uint16_t)(canMessage.id >> 5) & 0x1F)));
                    carMessage.setReceiverId((id_device_t)(canMessage.id & 0x1F));
                    if (canMessage.id & 0x400)
                        carMessage.setPrioritise(false);
                    else
                        carMessage.setPrioritise(true);

                    // Data-related IDs
                    carMessage.setComponentId((id_component_t)canMessage.data[0]);
                    carMessage.setMessageId(canMessage.data[1]);

                    // Length (well...)
                    carMessage.setLength(canMessage.len - 2);

                    // Data
                    for (uint8_t i = 0; i < carMessage.getLength(); i++) {
                        carMessage.set(canMessage.data[i+2], i);
                    }

                    returnVal = true;
                }
            } else if (canMessage.format == CANExtended) {
                // ID and Data-related IDs
                carMessage.setSenderId((id_device_t)(((uint32_t)(canMessage.id >> 18+5) & 0x1F)));
                carMessage.setReceiverId((id_device_t)(((uint32_t)(canMessage.id >> 18) & 0x1F)));
                carMessage.setComponentId((id_component_t)(((uint32_t)(canMessage.id >> 10) & 0xFF)));
                carMessage.setMessageId((uint8_t)(((uint32_t)(canMessage.id >> 2) & 0xFF)));
                if (canMessage.id & 0x10000000) {
                    carMessage.setPrioritise(false);
                } else {
                    carMessage.setPrioritise(true);
                }

                // Length (uff...)
                carMessage.setLength(canMessage.len);

                // Data
                for (uint8_t i = 0; i < canMessage.len; i++) {
                    carMessage.set(canMessage.data[i], i);
                }

                returnVal = true;
            }

            #if defined(CCAN_RECEIVING_DEBUG) && defined(MESSAGE_REPORT)
                printf("[CCAN]@_getCanMessage: Made CarMessage with component ID 0x%x and got message ID 0x%x in Format %i\n", carMessage.getComponentId(), canMessage.id, canMessage.format);
            #endif

            return returnVal;
        }

        /**
         * @brief Generate a CAN Message from a CarMessage
         * 
         * @param carMessage Reference to CarMessage
         * @param canMessage Reference to empty CANMessage
         * @return true If build was successfull
         * @return false If build failed
         */
        bool _getCanMessage(CarMessage &carMessage, CANMessage &canMessage) {
            bool returnVal = false;

            if (carMessage.getLength() < 7) {
                // Format for small Message
                canMessage.format = CANStandard;

                // ID
                canMessage.id = (uint16_t)(carMessage.getSenderId() & 0x1F) << 5;
                canMessage.id |= (uint16_t)carMessage.getReceiverId() & 0x1F;
                if (!carMessage.getPrioritise()) {
                    // -> Make message explicitly not prioritised
                    canMessage.id |= (uint16_t)0x400; // -> Binary 100 0000 0000
                }

                // Length (yes, it matters)
                canMessage.len = carMessage.getLength() + 2;

                // Data-related IDs
                canMessage.data[0] = carMessage.getComponentId();
                canMessage.data[1] = carMessage.getMessageId();

                // Data
                for (uint8_t i = 0; i < carMessage.getLength(); i++) {
                    canMessage.data[i+2] = carMessage.get(i);
                }

                returnVal = true;
            } else {
                // Format for long Message
                canMessage.format = CANExtended;

                // ID with Data-related IDs
                canMessage.id = (uint32_t)(carMessage.getSenderId() & 0x1F) << 18+5;
                canMessage.id |= (uint32_t)(carMessage.getReceiverId() & 0x1F) << 18;
                canMessage.id |= (uint32_t)(carMessage.getComponentId() & 0xFF) << 10;
                canMessage.id |= (uint32_t)(carMessage.getMessageId() & 0xFF) << 2;
                if (!carMessage.getPrioritise()) {
                    // -> Make message explicitly not prioritised
                    canMessage.id |= (uint32_t)0x10000000; // -> Binary 1 0000 0000 0000 0000 0000 0000 0000
                }

                // Length (yes, it matters, again)
                canMessage.len = carMessage.getLength();

                // Data
                for (uint8_t i = 0; i < carMessage.getLength(); i++) {
                    canMessage.data[i] = carMessage.get(i);
                }

                returnVal = true;
            }


            #if defined(CCAN_SENDING_DEBUG) && defined(MESSAGE_REPORT)
                printf("[CCAN]@_getCanMessage: Made CAN Message with component ID 0x%x and message ID 0x%x in Format %i\n", carMessage.getComponentId(), canMessage.id, canMessage.format);
            #endif

            return returnVal;
        }
};

#endif