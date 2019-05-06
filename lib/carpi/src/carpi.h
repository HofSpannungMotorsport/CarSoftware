/*
    This is the main include file of the cArPI

    With this, all components, services and programs are included in the actual CarSoftware.
*/

#define CARPI_VERSION "V0.1.1-P1 - Added Gas Curve"

// Prior include Platform-specific Components

#ifdef USE_MBED
    // Include Framework
    #include "mbed.h"
    #ifndef MESSAGE_REPORT
        #define MESSAGE_REPORT
        Serial pcSerial(USBTX, USBRX); // Connection to PC over Serial
    #endif

    // Communication
    #include "communication/CANService.h"
#endif

#ifdef USE_ARDUINO
    // Include Framework
    #include "Arduino.h"
    #include "crossplatform/arduinoToMbed/arduinoToMbed.h"
    HardwareSerial &pcSerial = Serial;
#endif

// ---------------------------------------------------------------------

// All Platform Components

// Communication
#include "communication/CarMessage.h"
#include "communication/componentIds.h"
#include "communication/deviceIds.h"

// Components
//   Interface
#include "components/interface/IComponent.h"
#include "components/interface/ICommunication.h"
#include "components/interface/IAnalogSensor.h"
#include "components/interface/IPump.h"
#include "components/interface/IHvEnabled.h"
//   Hardware
#include "components/hardware/HardwareAnalogSensor.h"
#include "components/hardware/HardwarePump.h"
#include "components/hardware/HardwareHvEnabled.h"
//   Software


// ---------------------------------------------------------------------


// After include Platform specific Components

#ifdef USE_MBED
    // Components
    //   Interface
    #include "components/interface/IButton.h"
    #include "components/interface/IBuzzer.h"
    #include "components/interface/IFan.h"
    #include "components/interface/ILed.h"
    #include "components/interface/IPedal.h"
    #include "components/interface/IMotorController.h"
    #include "components/interface/IRpmSensor.h"
    //   Hardware
    #include "components/hardware/HardwareBuzzer.h"
    #include "components/hardware/HardwarePwmBuzzer.h"
    #include "components/hardware/HardwareFan.h"
    #include "components/hardware/HardwareInterruptButton.h"
    #include "components/hardware/HardwareLed.h"
    #include "components/hardware/HardwareLedPwm.h"
    #include "components/hardware/HardwarePedal.h"
    #include "components/hardware/HardwareMotorController.h"
    #include "components/hardware/HardwareRpmSensor.h"
    //   Software
    #include "components/software/SoftwareButton.h"
    #include "components/software/SoftwareLed.h"
    #include "components/software/SoftwarePedal.h"
    #include "components/software/SoftwareRpmSensor.h"

    // Services
    #include "runable/services/IService.h"
    #include "runable/RunableList.h"
    #include "runable/RunableScheduler.h"
    #include "runable/services/SCar.h"
    #include "runable/services/SSpeed.h"

    // Programs
    #include "runable/programs/IProgram.h"
    #include "runable/programs/PBrakeLight.h"
    #include "runable/programs/PCooling.h"
    #include "runable/programs/PMotorController.h"
#endif

#ifdef USE_ARDUINO

#endif

// Include some Information about carpi (cross-platform)
#include <string>
class Carpi {
    public:
        Carpi() {
            // Print out the current Verison of Carpi
            printInfo();
        }

        void printInfo() {
            #ifdef USE_ARDUINO
                pcSerial.print("Carpi Version: "); pcSerial.println(_version.c_str());
                pcSerial.print("Environment: "); pcSerial.println(_environment.c_str());
            #endif

            #ifdef USE_MBED
                pcSerial.printf("Carpi Version: %s\nEnvironment: %s\n", _version.c_str(), _environment.c_str());
            #endif
        }

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