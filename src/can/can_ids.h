#ifndef CAN_IDS_H
#define CAN_IDS_H

#define PRIO_LEN 1
#define TELETYP_LEN 4
#define COMP_LEN 4
#define TELEFUNC_LEN 2

enum can_prio_t { 
    NORMAL = 0x1, 
    HIGH = 0x0 // high has to be zero because the first zero wins when a collision occurs
};

enum can_telegram_type_t {
    RPM = 0x5,
    PEDAL = 0x0,
    BUTTON = 0x3,
    COOLING = 0x2,
    GYROSCOPE = 0x8,
    ACCELERATION = 0x7,
    TEMPERATURE = 0x1,
    SUSPENSION_TRAVEL = 0x6,
    LED = 0x4,
    SYSTEM = 0xF
};

enum can_component_t {
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

    // Pedals
    PEDAL_BRAKE = 0x0,
    PEDAL_GAS = 0x1,

    // Buttons
    BTN_RESET = 0x0,
    BTN_START = 0x1,

    // LEDs
    LED_ERROR = 0x0, //red LED dashboard
    LED_WARNING = 0x1, // yellow LED dashboard
    LED_RTD = 0x2, //ready-to-drive LED, green one

    // Cooling
    COOLING_PUMP = 0x0,
    COOLING_FANS = 0x1,
    
    // System messages
    SYSTEM_INIT_REQUEST = 0x0,
    SYSTEM_INIT_RESPONSE = 0x1,
    SYSTEM_START_CALIBRATION = 0x2,
    SYSTEM_FINISH_CALIBRATION = 0x3,
    SYSTEM_CANCEL_CALIBRATION = 0x4
};

uint8_t getCompId(can_telegram_type_t teletype, can_component_t comp) {
    return (teletype << 4) & comp; 
}

uint16_t getMessageId(uint8_t prio, can_telegram_type_t teletype, can_component_t comp, uint8_t func) {
    // TODO calculate func
    return (prio << 10) & getConpId(teletype, comp);
}

#endif