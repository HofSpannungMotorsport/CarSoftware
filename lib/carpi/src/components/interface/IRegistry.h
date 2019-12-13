#ifndef IREGISTRY_H
#define IREGISTRY_H

#include "communication/SelfSyncable.h"

typedef uint16_t registry_index_t;

enum float_registry_index_t : registry_index_t {
    // Components
    //   Alive
    ALIVE_SIGNAL_REFRESH_RATE = 0x0,
    ALIVE_SIGNAL_SEND_RATE = 0x1,

    //   Button
    BUTTON_DEBOUNCE_TIME = 0x2,
    BUTTON_LONG_CLICK_TIME = 0x3,

    //   Buzzer
    STD_BUZZER_ON_OFF_TIME = 0x4,
    STD_BUZZER_FAST_HIGH_LOW_TIME = 0x5,
    STD_BUZZER_HIGH_LOW_TIME = 0x6,
    STD_BUZZER_PWM_HIGH_TO_LOW_RATIO = 0x7,

    //   LED
    LED_BLINKING_SLOW_HZ = 0x8,
    LED_BLINKING_NORMAL_HZ = 0x9,
    LED_BLINKING_FAST_HZ = 0xA,

    //   Motor Controller
    STD_MC_SPEED_REFRESH_TIME = 0xB,
    STD_MC_CURRENT_REFRESH_TIME = 0xC,
    STD_MC_CURRENT_DEVICE_REFRESH_TIME = 0xD,
    STD_MC_MOTOR_TEMP_REFRESH_TIME = 0xE,
    STD_MC_CONTROLLER_TEMP_REFRESH_TIME = 0xF,
    STD_MC_AIR_TEMP_REFRESH_TIME = 0x10,

    //   Pedal
    STD_PEDAL_MAX_DEVIANCE = 0x11,
    STD_PEDAL_MAPPED_BOUNDARY_PERCENTAGE = 0x12,
    STD_PEDAL_CALIBRATION_REFRESH_TIME = 0x13,
    STD_PEDAL_THRESHHOLD = 0x14,


    // Programs
    //   PBrakeLight
    BRAKE_LIGHT_LOWER_THRESHHOLD = 0x15,
    BRAKE_LIGHT_UPPER_THRESHHOLD = 0x16,

    //   PCooling
    COOLING_FAN_OFF_UNTIL_SPEED = 0x17,
    COOLING_FAN_ON_UNITL_SPEED = 0x18,
    COOLING_MOTOR_TEMP_START_PUMP = 0x19,
    COOLING_MOTOR_TEMP_FULL_PUMP = 0x1A,
    COOLING_CONTROLLER_TEMP_START_PUMP = 0x1B,
    COOLING_CONTROLLER_TEMP_FULL_PUMP = 0x1C,
    COOLING_MAX_MOTOR_TEMP = 0x1D,
    COOLING_MAX_CONTROLLER_TEMP = 0x1E,

    //   PMotorController
    PMC_MAX_POWER = 0x1F,
    PMC_POWER_SET_ON_MC = 0x20,
    PMC_AGE_LIMIT = 0x21,
    PMC_BRAKE_POWER_LOCK_THRESHHOLD = 0x22,
    PMC_GAS_PEDAL_PRIME_MIN = 0x23,
    PMC_HARD_BRAKE_THRESHHOLD = 0x24,
    PMC_HARD_BRAKE_PRESSURE = 0x25,
    PMC_HARD_BRAKE_CUTOFF_TIME = 0x26,
    PMC_HARD_BRAKE_CUTOFF_APPS_POSITION = 0x27,
    PMC_HARD_BRAKE_CUTOFF_POWER = 0x28,


    // Services
    //   SCar
    STD_SCAR_STARTUP_ANIMATION_SPEED = 0x29,
    STD_SCAR_STARTUP_ANIMATION_WAIT_AFTER = 0x2A,
    SCAR_BRAKE_START_THRESHHOLD = 0x2B,
    SCAR_HV_ENABLED_BEEP_TIME = 0x2C,

    //   SSpeed
    SSPEED_SPEED_DEVIANCE_THRESHHOLD = 0x2D,
    SSPEED_MAX_SPEED_DEVIANCE = 0x2E,
    SSPEED_DISTANCE_PER_REVOLUTION = 0x2F,
    SSPEED_MOTOR_TO_WHEEL_RATIO = 0x30,
    SSPEED_THROTTLE_FROM = 0x31,
    SSPEED_THROTTLE_TO = 0x32,
    SSPEED_THROTTLE_END_VALUE = 0x33
};
const registry_index_t float_registry_size = 0x34;

enum uint8_registry_index_t : registry_index_t {
    STD_SCAR_STARTUP_ANIMATION_PLAYBACKS = 0x0
};
const registry_index_t uint8_registry_size = 0x1;

enum uint16_registry_index_t : registry_index_t {
    // Components
    //   Analog Sensor
    STD_ANALOG_SENSOR_OUT_OF_BOUNDARY_TIME_LIMIT = 0x0,
    STD_ANALOG_SENSOR_OUT_OF_BOUNDARY_TIME_LIMIT_RAW = 0x1,

    //   Buzzer
    STD_BUZZER_HIGH_HZ = 0x2,

    //   Pedal
    STD_PEDAL_GAS_1_MIN = 0x3,
    STD_PEDAL_GAS_1_MAX = 0x4,
    STD_PEDAL_GAS_2_MIN = 0x5,
    STD_PEDAL_GAS_2_MAX = 0x6,
    STD_PEDAL_BRAKE_MIN = 0x7,
    STD_PEDAL_BRAKE_MAX = 0x8,
    STD_PEDAL_MAX_DEVIANCE_TIME = 0x9,
    STD_PEDAL_MAX_OUT_OF_BOUNDARY_TIME = 0xA,
    STD_PEDAL_CALIBRATION_MIN_DEVIANCE = 0xB,
    STD_PEDAL_CALIBRATION_MAX_DEVIANCE = 0xC,
    STD_PEDAL_ANALOG_LOWER_BOUNDARY = 0xD,
    STD_PEDAL_ANALOG_UPPER_BOUNDARY = 0xE
};
const registry_index_t uint16_registry_size = 0xF;

enum uint32_registry_index_t : registry_index_t {

};
const registry_index_t uint32_registry_size = 0;

enum int8_registry_index_t : registry_index_t {

};
const registry_index_t int8_registry_size = 0;

enum int16_registry_index_t : registry_index_t {

};
const registry_index_t int16_registry_size = 0;

enum int32_registry_index_t : registry_index_t {

};
const registry_index_t int32_registry_size = 0;

enum bool_registry_index_t : registry_index_t {

};
const registry_index_t bool_registry_size = 0;


enum registry_message_command_t : uint8_t {
    REGISTRY_SET_NOT_READY = 0x0,
    REGISTRY_SET_READY,
    REGISTRY_CRC,
    REGISTRY_CRC_NOT_MATCHING,
    REGISTRY_TYPE_FLOAT,
    REGISTRY_TYPE_UINT8,
    REGISTRY_TYPE_UINT16,
    REGISTRY_TYPE_UINT32,
    REGISTRY_TYPE_INT8,
    REGISTRY_TYPE_INT16,
    REGISTRY_TYPE_INT32,
    REGISTRY_TYPE_BOOL
};


enum registry_error_type_t : status_t {
    REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS = 0x1,
    REGISTRY_ERROR_RECEIVED_TOO_LONG_MSG = 0x2,
    REGISTRY_ERROR_UNKNOWN_MSG_RECEIVED = 0x4
};


class IRegistry : public SelfSyncable {
    public:
        IRegistry() {
            setComponentType(COMPONENT_SYSTEM);
            setObjectType(OBJECT_INTERNAL);
        }

        // Registry Ready?
        virtual bool getReady() = 0;
        virtual void setReady(bool ready) = 0;

        // CRC
        virtual uint16_t getCrc() = 0;

        // Getter
        // float
        virtual float getFloat(float_registry_index_t index) = 0;
        
        // int
        virtual uint8_t getUInt8(uint8_registry_index_t index) = 0;
        virtual uint16_t getUInt16(uint16_registry_index_t index) = 0;
        virtual uint32_t getUInt32(uint32_registry_index_t index) = 0;
        virtual int8_t getInt8(int8_registry_index_t index) = 0;
        virtual int16_t getInt16(int16_registry_index_t index) = 0;
        virtual int32_t getInt32(int32_registry_index_t index) = 0;

        // bool
        virtual bool getBool(bool_registry_index_t index) = 0;


        // Setter
        // float
        virtual void setFloat(float_registry_index_t index, float value) = 0;
        
        // int
        virtual void setUInt8(uint8_registry_index_t index, uint8_t value) = 0;
        virtual void setUInt16(uint16_registry_index_t index, uint16_t value) = 0;
        virtual void setUInt32(uint32_registry_index_t index, uint32_t value) = 0;
        virtual void setInt8(int8_registry_index_t index, int8_t value) = 0;
        virtual void setInt16(int16_registry_index_t index, int16_t value) = 0;
        virtual void setInt32(int32_registry_index_t index, int32_t value) = 0;

        // bool
        virtual void setBool(bool_registry_index_t index, bool value) = 0;
};

#endif // IREGISTRY_H