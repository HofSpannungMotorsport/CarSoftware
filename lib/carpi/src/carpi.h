#ifndef CARPI_H
#define CARPI_H

/*
    This is the main include file of the cArPI

    With this, all components, services and programs are included in the actual CarSoftware.
*/

#define CARPI_VERSION "V0.2.0-P5 - Alive & Report & Bugfix & Registry"
#define MESSAGE_REPORT

// Prior include Platform-specific Components

#ifdef USE_MBED
    // Include Framework
    #include "mbed.h"
    #include "Steroido.h"
#endif

#if defined(USE_ARDUINO) || defined(USE_TEENSYDUINO)
    // Include Framework
    #include "Arduino.h"

    #ifdef USE_TEENSYDUINO
        #define TEENSY
    #endif

    #include "Steroido.h"
#endif

// ---------------------------------------------------------------------

// All Platform Components

// Communication
#include "communication/Sync.h"
#include "communication/CarMessage.h"
#include "communication/componentIds.h"
#include "communication/deviceIds.h"
#include "communication/IChannel.h"
#include "communication/SelfSyncable.h"

#ifdef STEROIDO_DEVICE_CAN
    #include "communication/CCan.h"
#endif

// Components
//   Interface
#include "components/interface/IRegistry.h"
#include "components/interface/IComponent.h"
#include "components/interface/IAnalogSensor.h"
#include "components/interface/IPump.h"
#include "components/interface/IHvEnabled.h"
#include "components/interface/IAlive.h"
#include "components/interface/IButton.h"
#include "components/interface/IBuzzer.h"
#include "components/interface/IFan.h"
#include "components/interface/ILed.h"
#include "components/interface/IPedal.h"
#include "components/interface/IMotorController.h"
#include "components/interface/IRpmSensor.h"
#include "components/interface/ISDCard.h"

//   Hardware
#include "components/hardware/HardwareAnalogSensor.h"
#include "components/hardware/HardwarePump.h"
#include "components/hardware/HardwareAlive.h"
#include "components/hardware/HardwareBuzzer.h"
#include "components/hardware/HardwareFan.h"
#include "components/hardware/HardwareLed.h"
#include "components/hardware/HardwareLedPwm.h"
#include "components/hardware/HardwarePedal.h"

#ifdef USE_MBED
    #include "components/hardware/HardwareHvEnabled.h"
    #include "components/hardware/HardwareInterruptButton.h"
    #include "components/hardware/HardwarePwmBuzzer.h"
    #include "components/hardware/HardwareRpmSensor.h"
    #include "components/hardware/HardwareMotorController.h"
    #include "components/hardware/HardwareSDCard.h"
#endif

//   Software
#include "components/software/SoftwareAlive.h"
#include "components/software/SoftwareButton.h"
#include "components/software/SoftwareLed.h"
#include "components/software/SoftwarePedal.h"
#include "components/software/SoftwareRpmSensor.h"

//   Internal
#include "components/internal/InternalRegistry.h"
#include "components/internal/InternalRegistryHardStorage.h"

// Services
#include "runable/services/IService.h"
#include "runable/RunableList.h"
#include "runable/RunableScheduler.h"

#ifdef USE_MBED
    #include "runable/services/SCar.h"
    #include "runable/services/SSpeed.h"
#endif


// Programs
#include "runable/programs/IProgram.h"

#ifdef USE_MBED
    #include "runable/programs/PCockpitIndicator.h"
    #include "runable/programs/PBrakeLight.h"
    #include "runable/programs/PCooling.h"
    #include "runable/programs/PMotorController.h"
    #include "runable/programs/PLogger.h"
#endif


// Include some Information about carpi (cross-platform)
#include <string>
class Carpi {
    public:
        Carpi() {
            // Print out the current Verison of Carpi
            #ifdef MESSAGE_REPORT
            #ifdef USE_ARDUINO
                Serial.begin(9600);
            #endif
            printInfo();
            #endif
        }

        #ifdef MESSAGE_REPORT
        void printInfo() {
                printf("Carpi Version: %s\nEnvironment: %s\n", _version.c_str(), _environment.c_str());
        }
        #endif

        string getVersion() {
            return _version;
        }

        string getEnvironment() {
            return _environment;
        }

    private:
        string _version = CARPI_VERSION;
        string _environment = ENVIRONMENT;
};

#endif // CARPI_H