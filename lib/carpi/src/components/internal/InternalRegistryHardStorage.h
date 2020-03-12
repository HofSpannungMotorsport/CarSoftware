#ifndef INTERNAL_REGISTRY_HARD_STORAGE_H
#define INTERNAL_REGISTRY_HARD_STORAGE_H

#include "../interface/IRegistry.h"
#include "communication/Sync.h"

// static -> only in this file needed (and visible)
// const -> values will not be changed -> const makes them also only stored in Program-Memory, not RAM

static const float _floatRegistryStorage[] = {
    // Components
    //   Alive
    /* ALIVE_SIGNAL_REFRESH_RATE */ 0.05,
    /* ALIVE_SIGNAL_SEND_RATE */ 0.025,

    //   Button
    /* BUTTON_DEBOUNCE_TIME */ 0.010,
    /* BUTTON_LONG_CLICK_TIME */ 800,

    //   Buzzer
    /* STD_BUZZER_ON_OFF_TIME */ 0.5,
    /* STD_BUZZER_FAST_HIGH_LOW_TIME */ 0.075,
    /* STD_BUZZER_HIGH_LOW_TIME */ 0.6,
    /* STD_BUZZER_PWM_HIGH_TO_LOW_RATIO */ 0.5,

    //   LED
    /* LED_BLINKING_SLOW_HZ */ 0.66,
    /* LED_BLINKING_NORMAL_HZ */ 3,
    /* LED_BLINKING_FAST_HZ */ 6,

    //   Motor Controller
    /* STD_MC_SPEED_REFRESH_TIME */ 240,
    /* STD_MC_CURRENT_REFRESH_TIME */ 240,
    /* STD_MC_CURRENT_DEVICE_REFRESH_TIME */ 240,
    /* STD_MC_MOTOR_TEMP_REFRESH_TIME */ 12,
    /* STD_MC_CONTROLLER_TEMP_REFRESH_TIME */ 12,
    /* STD_MC_AIR_TEMP_REFRESH_TIME */ 6,

    //   Pedal
    /* STD_PEDAL_MAX_DEVIANCE */ 0.1,
    /* STD_PEDAL_MAPPED_BOUNDARY_PERCENTAGE */ 0.1,
    /* STD_PEDAL_CALIBRATION_REFRESH_TIME */ 0.01,
    /* STD_PEDAL_THRESHHOLD */ 0.15,


    // Programs
    //   PBrakeLight
    /* BRAKE_LIGHT_LOWER_THRESHHOLD */ 0.20,
    /* BRAKE_LIGHT_UPPER_THRESHHOLD */ 0.35,

    //   PCooling
    /* COOLING_FAN_OFF_UNTIL_SPEED */ 7,
    /* COOLING_FAN_ON_UNITL_SPEED */ 5,
    /* COOLING_MOTOR_TEMP_START_PUMP */ 30,
    /* COOLING_MOTOR_TEMP_FULL_PUMP */ 60,
    /* COOLING_CONTROLLER_TEMP_START_PUMP */ 30,
    /* COOLING_CONTROLLER_TEMP_FULL_PUMP */ 40,
    /* COOLING_MAX_MOTOR_TEMP */ 110,
    /* COOLING_MAX_CONTROLLER_TEMP */ 65,

    //   PMotorController
    /* PMC_MAX_POWER */ 80,
    /* PMC_POWER_SET_ON_MC */ 80,
    /* PMC_AGE_LIMIT */ 3,
    /* PMC_BRAKE_POWER_LOCK_THRESHHOLD */ 0.4,
    /* PMC_GAS_PEDAL_PRIME_MIN */ 0.05,
    /* PMC_HARD_BRAKE_THRESHHOLD */ 0.75,
    /* PMC_HARD_BRAKE_PRESSURE */ 30,
    /* PMC_HARD_BRAKE_CUTOFF_TIME */ 0.5,
    /* PMC_HARD_BRAKE_CUTOFF_APPS_POSITION */ 0.25,
    /* PMC_HARD_BRAKE_CUTOFF_POWER */ 25,


    // Services
    //   SCar
    /* STD_SCAR_STARTUP_ANIMATION_SPEED */ 0.075,
    /* STD_SCAR_STARTUP_ANIMATION_WAIT_AFTER */ 0.25,
    /* SCAR_BRAKE_START_THRESHHOLD */ 0.6,
    /* SCAR_HV_ENABLED_BEEP_TIME */ 2.2,

    //   SSpeed
    /* SSPEED_SPEED_DEVIANCE_THRESHHOLD */ 3,
    /* SSPEED_MAX_SPEED_DEVIANCE */ 0.1,
    /* SSPEED_DISTANCE_PER_REVOLUTION */ 1.4451326206513048896928159563086,
    /* SSPEED_MOTOR_TO_WHEEL_RATIO */ (1.0/3.6),
    /* SSPEED_THROTTLE_FROM */ 40,
    /* SSPEED_THROTTLE_TO */ 105,
    /* SSPEED_THROTTLE_END_VALUE */ 0.65
};

static const uint8_t _uint8RegistryStorage[] = {
    /* STD_SCAR_STARTUP_ANIMATION_PLAYBACKS */ 2
};

static const uint16_t _uint16RegistryStorage[] = {
    // Components
    //   Analog Sensor
    /* STD_ANALOG_SENSOR_OUT_OF_BOUNDARY_TIME_LIMIT */ 99,
    /* STD_ANALOG_SENSOR_OUT_OF_BOUNDARY_TIME_LIMIT_RAW */ 89,

    //   Buzzer
    /* STD_BUZZER_HIGH_HZ */ 300,

    //   Pedal
    /* STD_PEDAL_GAS_1_MIN */ 18196,
    /* STD_PEDAL_GAS_1_MAX */ 45250,
    /* STD_PEDAL_GAS_2_MIN */ 12663,
    /* STD_PEDAL_GAS_2_MAX */ 42444,
    /* STD_PEDAL_BRAKE_MIN */ 12684,
    /* STD_PEDAL_BRAKE_MAX */ 20113,
    /* STD_PEDAL_MAX_DEVIANCE_TIME */ 100,
    /* STD_PEDAL_MAX_OUT_OF_BOUNDARY_TIME */ 100,
    /* STD_PEDAL_CALIBRATION_MIN_DEVIANCE */ 500,
    /* STD_PEDAL_CALIBRATION_MAX_DEVIANCE */ 50000,
    /* STD_PEDAL_ANALOG_LOWER_BOUNDARY */ 655,
    /* STD_PEDAL_ANALOG_UPPER_BOUNDARY */ 64880
};

static const uint32_t _uint32RegistryStorage[] = {};

static const int8_t _int8RegistryStorage[] = {};

static const int16_t _int16RegistryStorage[] = {};

static const int32_t _int32RegistryStorage[] = {};

static const bool _boolRegistryStorage[] = {};


class InternalRegistryHardStorage {
    public:
        // Loads in the Values stored in the InternalRegistryHardStorage. If Sync is given, it will wait till the values are sent, then continue with sending the next one.
        static void loadIn(IRegistry &registry, Sync *sync = nullptr) {
            // load Type with                           registerStorage                                  size of it                            registry  function pointer to set   Syncer...
            _loadType<float, float_registry_index_t>(_floatRegistryStorage, (registry_index_t)(sizeof(_floatRegistryStorage) / sizeof(float)), registry, &IRegistry::setFloat, sync);
            _loadType<uint8_t, uint8_registry_index_t>(_uint8RegistryStorage, (registry_index_t)(sizeof(_uint8RegistryStorage) / sizeof(uint8_t)), registry, &IRegistry::setUInt8, sync);
            _loadType<uint16_t, uint16_registry_index_t>(_uint16RegistryStorage, (registry_index_t)(sizeof(_uint16RegistryStorage) / sizeof(uint16_t)), registry, &IRegistry::setUInt16, sync);
            _loadType<uint32_t, uint32_registry_index_t>(_uint32RegistryStorage, (registry_index_t)(sizeof(_uint32RegistryStorage) / sizeof(uint32_t)), registry, &IRegistry::setUInt32, sync);
            _loadType<int8_t, int8_registry_index_t>(_int8RegistryStorage, (registry_index_t)(sizeof(_int8RegistryStorage) / sizeof(int8_t)), registry, &IRegistry::setInt8, sync);
            _loadType<int16_t, int16_registry_index_t>(_int16RegistryStorage, (registry_index_t)(sizeof(_int16RegistryStorage) / sizeof(int16_t)), registry, &IRegistry::setInt16, sync);
            _loadType<int32_t, int32_registry_index_t>(_int32RegistryStorage, (registry_index_t)(sizeof(_int32RegistryStorage) / sizeof(int32_t)), registry, &IRegistry::setInt32, sync);
            _loadType<bool, bool_registry_index_t>(_boolRegistryStorage, (registry_index_t)(sizeof(_boolRegistryStorage) / sizeof(bool)), registry, &IRegistry::setBool, sync);

            registry.setReady(true);
        }
    
    private:
        template <typename T, typename registry_index_type_t>
        static void _loadType(const T registerStorage[], registry_index_t size, IRegistry &registry, void (IRegistry::*setPointer)(registry_index_type_t, T), Sync *sync) {
            for (registry_index_t i = 0; i < size; i++) {
                (registry.*setPointer)((registry_index_type_t)i, registerStorage[i]);

                if (sync != nullptr) {
                    while(sync->messagesInOutBuffer()) {
                        sync->run();
                    }
                }
            }
        }
};


#endif // INTERNAL_REGISTRY_HARD_STORAGE_H