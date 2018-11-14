#ifndef CAN_IDS_H
#define CAN_IDS_H

#include <stdint.h>

#define PRIORITY_LENGTH 1
#define TELEGRAM_TYPE_LENGTH 4
#define COMPONENT_LENGTH 4
#define OBJECT_TYPE_LENGTH 2

/*
    CAN-Telegram-ID for the Master-Slave CAN-Bus (11 Bit long):
    The first zero in the whole ID wins -> higher Priority.
    That means all IDs have to be sorted by Priority.

    Bits:
    1   2 3 4 5   6 7 8 9   10 11

    1.      Bit: Priority -> make Message important
    2. - 5. Bit: Telegram type (Pedal, Button, LED...)
    6. - 9. Bit: Component type (Break Pedal, Gas Pedal...)
    10.-11. Bit: Object Type (Hardware, Software...)
*/

enum can_priority_t : uint8_t { 
    NORMAL = 0x1, 
    HIGH =   0x0 // high has to be zero because the first zero wins when a collision occurs
};

enum can_telegram_type_t : uint8_t {
    // More important Values have to be smaller
    PEDAL =             0x0,
    RPM_SENSOR =        0x1,
    TEMPERATURE =       0x2,
    COOLING =           0x3,
    BUTTON =            0x4,
    LED =               0x5,
    RPM =               0x6,
    SUSPENSION_TRAVEL = 0x7,
    ACCELERATION =      0x8,
    GYROSCOPE =         0x9,
    SYSTEM =            0xF
};

enum can_component_t : uint8_t {
    // Pedals
    PEDAL_BRAKE = 0x0,
    PEDAL_GAS =   0x1,

    // RPM Sensors
    RPM_FRONT_LEFT =  0x0,
    RPM_FRONT_RIGHT = 0x1,
    RPM_REAR_LEFT =   0x2,
    RPM_REAR_RIGHT =  0x3,

    // Cooling
    COOLING_PUMP = 0x0,
    COOLING_FANS = 0x1,

    // Buttons
    BUTTON_RESET = 0x0,
    BUTTON_START = 0x1,

    // LEDs
    LED_ERROR =          0x0, //red LED dashboard
    LED_ISSUE =          0x1, // yellow LED dashboard
    LED_READY_TO_DRIVE = 0x2, // greed LED dashboard
    LED_MASTER =         0x3, // green LED on the board PCB (master)
    LED_DASHBOARD =      0x4, // green LED on the board PCB (dashboard)
    LED_PEDAL =          0x5, // green LED on the board PCB (pedal)

    // RPM sensors
    RPM_TIRE_FR = 0x0,
    RPM_TIRE_FL = 0x1,
    RPM_TIRE_RR = 0x2,
    RPM_TIRE_RL = 0x3,

    // Suspension travel sensors
    STS_FR = 0x0,
    STS_FL = 0x1,
    STS_RR = 0x2,
    STS_RL = 0x3,
    
    // System messages
    SYSTEM_MASTER =    0x0,
    SYSTEM_DASHBOARD = 0x1,
    SYSTEM_PEDAL =     0x2
    /*
    SYSTEM_INIT_REQUEST = 0x0,
    SYSTEM_INIT_RESPONSE = 0x1,
    SYSTEM_START_CALIBRATION = 0x2,
    SYSTEM_FINISH_CALIBRATION = 0x3,
    SYSTEM_CANCEL_CALIBRATION = 0x4
    */
};

typedef uint8_t can_object_type_datatype_t;
enum can_object_type_t : can_object_type_datatype_t {
    UNDEFINED_OBJECT = 0x0,
    HARDWARE_OBJECT = 0x1,
    SOFTWARE_OBJECT = 0x2,
    DEVICE_CONTROLLER = 0x3
};

typedef uint8_t component_id_t;
typedef uint16_t message_id_t;

namespace ID {
    component_id_t getComponentId(can_telegram_type_t teletype, can_component_t comp) {
        component_id_t componentID = 0;
        componentID = (teletype << COMPONENT_LENGTH) | comp;
        return componentID; 
    }

    message_id_t getMessageId(can_priority_t priority, component_id_t componentId, uint8_t objectType) {
        message_id_t messageID = 0;
        messageID |= (priority & 0xF) << (OBJECT_TYPE_LENGTH + COMPONENT_LENGTH + TELEGRAM_TYPE_LENGTH); // Put priority at the beginning
        messageID |= (componentId & 0xF) << OBJECT_TYPE_LENGTH; // Add Component ID
        messageID |= (objectType & 0x3);
        return messageID;
    }

    message_id_t getMessageId(can_priority_t priority, can_telegram_type_t telegramType, can_component_t component, uint8_t objectType) {
        return getMessageId(priority, getComponentId(telegramType, component), objectType);
    }
};

#endif