#ifndef COMPONENTIDS_H
#define COMPONENTIDS_H

#define COMPONENT_TYPE_LENGTH 4
#define COMPONENT_SUB_ID_LENGTH 4

/*
    Component IDs

    Bits:
    1 2 3 4   5 6 7 8

    1. - 4. Bit: Component type (Pedal, Button, LED...)
    5. - 8. Bit: Component subId (Break Pedal, Gas Pedal...)
*/

enum id_component_type_t : uint8_t {
    // More important Values have to be smaller
    COMPONENT_MOTOR =             0x1,
    COMPONENT_PEDAL =             0x2,
    COMPONENT_RPM_SENSOR =        0x3,
    COMPONENT_TEMPERATURE =       0x4,
    COMPONENT_COOLING =           0x5,
    COMPONENT_BUTTON =            0x6,
    COMPONENT_LED =               0x7,
    COMPONENT_SUSPENSION_TRAVEL = 0x8,
    COMPONENT_ACCELERATION =      0x9,
    COMPONENT_GYROSCOPE =         0xA,
    COMPONENT_BUZZER =            0xB,
    COMPONENT_SYSTEM =            0xF
};

enum id_sub_component_t : uint8_t {
    // Motor
    COMPONENT_MOTOR_MAIN = 0x0,

    // Pedals
    COMPONENT_PEDAL_BRAKE = 0x0,
    COMPONENT_PEDAL_GAS =   0x1,

    // RPM Sensors
    COMPONENT_RPM_FRONT_LEFT =  0x0,
    COMPONENT_RPM_FRONT_RIGHT = 0x1,
    COMPONENT_RPM_REAR_LEFT =   0x2,
    COMPONENT_RPM_REAR_RIGHT =  0x3,

    // Cooling
    COMPONENT_COOLING_PUMP = 0x0,
    COMPONENT_COOLING_FAN  = 0x1,

    // Buttons
    COMPONENT_COMPONENT_BUTTON_RESET = 0x0,
    BUTTON_START = 0x1,

    // LEDs
    COMPONENT_LED_BRAKE =          0x0, // The braking light on the back of the car
    COMPONENT_LED_ERROR =          0x1, // red LED dashboard
    COMPONENT_LED_ISSUE =          0x2, // yellow LED dashboard
    COMPONENT_LED_READY_TO_DRIVE = 0x3, // greed LED dashboard
    COMPONENT_LED_MASTER_RED =     0x4, // red LED on the board PCB (master)
    COMPONENT_LED_MASTER_BLUE =    0x5, // blue LED on the board PCB (master)
    COMPONENT_LED_MASTER_GREEN =   0x6, // green LED on the board PCB (master)
    COMPONENT_LED_DASHBOARD =      0x7, // green LED on the board PCB (dashboard)
    COMPONENT_LED_PEDAL =          0x8, // green LED on the board PCB (pedal)

    // Suspension travel sensors
    COMPONENT_SUSPENSION_TRAVEL_FR = 0x0,
    COMPONENT_SUSPENSION_TRAVEL_FL = 0x1,
    COMPONENT_SUSPENSION_TRAVEL_RR = 0x2,
    COMPONENT_SUSPENSION_TRAVEL_RL = 0x3,

    // Buzzer
    COMPONENT_BUZZER_STARTUP = 0x0,
    
    // System messages
    COMPONENT_SYSTEM_BMS =              0x0,
    COMPONENT_SYSTEM_MASTER =           0x1,
    COMPONENT_SYSTEM_DASHBOARD =        0x2,
    COMPONENT_SYSTEM_PEDAL =            0x3,
    COMPONENT_SYSTEM_MOTOR_CONTROLLER = 0x4,
    COMPONENT_SYSTEM_SPEED =            0x5,
    COMPONENT_SYSTEM_COOLING =          0x6,
    COMPONENT_SYSTEM_HV_ENABLED =       0x7
};

typedef uint8_t id_component_t;

namespace componentId {

    /*
        Get the whole ID for a Component by giving the componentType and componentSubId of the wanted Component
    */
    id_component_t getComponentId(id_component_type_t componentType, id_sub_component_t componentSubId) {
        id_component_t componentId;

        componentId = ((componentType & 0xF) << COMPONENT_SUB_ID_LENGTH);
        componentId |= (somponentSubId & 0xF);

        return componentId;
    }
}; // componentId

#endif // COMPONENTIDS_H