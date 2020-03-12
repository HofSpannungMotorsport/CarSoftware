#ifndef DEVICEIDS_H
#define DEVICEIDS_H

/*
    All IDs for the Devices in the whole System.
    
    Because of the focus on CAN for efficient messageing systems,
    the ID is limited to 5 bit. a Max of 30 Devices can be in
    the System.

    If the need is there, the ID will be set to 6 bit using extended
    CAN Messages. This just has to be changed for CAN, please keep
    in mind for other Channels at building time!
*/

enum id_device_type_t : uint8_t {
    DEVICE_TYPE_SENDER = 0x0,
    DEVICE_TYPE_RECEIVER = 0x1
};

enum id_device_t : uint8_t {
    DEVICE_NOT_SET =     0x0,
    DEVICE_ALL =         0x1,
    DEVICE_PEDAL =       0x2,
    DEVICE_BMS =         0x3,
    DEVICE_DASHBOARD =   0x4,
    DEVICE_MASTER =      0x5,
    DEVICE_DISPLAY =     0x6,
    DEVICE_LORA_CAR =    0x7,
    DEVICE_LORA_GROUND = 0x8
};

typedef uint16_t id_message_header_t;

namespace deviceId {
    id_message_header_t getMessageHeader(id_device_t senderDeviceId, id_device_t receiverDeviceId){
        id_message_header_t messageHeader;

        messageHeader = (((id_message_header_t)senderDeviceId & 0x1F) << 5);
        messageHeader |= (id_message_header_t)receiverDeviceId & 0x1F;

        return messageHeader;
    }

    id_device_t getDeviceIdFromMessageHeader(id_message_header_t messageHeader, id_device_type_t deviceType) {
        if (deviceType == DEVICE_TYPE_RECEIVER) {
            messageHeader &= 0x1F;
        } else if (deviceType == DEVICE_TYPE_SENDER) {
            messageHeader = (messageHeader >> 5) & 0x1F;
        } else return DEVICE_NOT_SET;

        return (id_device_t)messageHeader;
    }
}; // deviceId

#endif // DEVICEIDS_H