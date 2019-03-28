#ifndef DEVICEIDS_H
#define DEVICEIDS_H

/*
    All IDs for the Devices in the whole System.
    
    Because of the focus on CAN for effizient messageing systems,
    the ID is limited to 5 bit. a Max of 32 Devices can be in
    the System.
*/

enum id_device_t : uint8_t {
    DEVICE_PEDAL =     0x0,
    DEVICE_BMS =       0x1,
    DEVICE_DASHBOARD = 0x2,
    DEVICE_MASTER =    0x3,
    DEVICE_DISPLAY =   0x4
};

typedef uint16_t id_message_header_t;

namespace deviceId {
    id_message_header_t getMessageHeader(id_device_t senderDeviceId, id_device_t receiverDeviceId){
        id_message_header_t messageHeader;

        messageHeader = (senderDeviceId << 5);
        messageHeader |= receiverDeviceId;

        return messageHeader;
    }
}; // deviceId

#endif // DEVICEIDS_H