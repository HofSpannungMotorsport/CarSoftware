/*
    This is the main include file of the cArPI

    With this, all components, services and programs are included in the actual CarSoftware.
*/

// At first, add all components

#ifdef USE_MBED
    // Framework
    #include "mbed.h"

    #ifndef MESSAGE_REPORT
        #define MESSAGE_REPORT
        Serial pcSerial(USBTX, USBRX); // Connection to PC over Serial
    #endif

    // Communication
    #include "communication/can_config.h"
    #include "communication/CANService.h"

    // Components
    //   Message Handler
    #include "components/handler/IMessageHandler.h"
    #include "components/handler/ButtonMessageHandler.h"
    #include "components/handler/LEDMessageHandler.h"
    #include "components/handler/PedalMessageHandler.h"
    #include "components/handler/RpmSensorMessageHandler.h"
    //   Interface
    #include "components/interface/IID.h"
    #include "components/interface/IAnalogSensor.h"
    #include "components/interface/IButton.h"
    #include "components/interface/IBuzzer.h"
    #include "components/interface/IFan.h"
    #include "components/interface/ILed.h"
    #include "components/interface/IMotorController.h"
    #include "components/interface/IPedal.h"
    #include "components/interface/IPump.h"
    #include "components/interface/IRpmSensor.h"
    //   Hardware
    #include "components/hardware/HardwareAnalogSensor.h"
    #include "components/hardware/HardwareBuzzer.h"
    #include "components/hardware/HardwareFan.h"
    #include "components/hardware/HardwareInterruptButton.h"
    #include "components/hardware/HardwareLed.h"
    #include "components/hardware/HardwareLedPwm.h"
    #include "components/hardware/HardwareMotorController.h"
    #include "components/hardware/HardwarePedal.h"
    #include "components/hardware/HardwarePump.h"
    #include "components/hardware/HardwarePwmBuzzer.h"
    #include "components/hardware/HardwareRpmSensor.h"
    //   Software
    #include "components/software/SoftwareButton.h"
    #include "components/software/SoftwareLed.h"
    #include "components/software/SoftwarePedal.h"
    #include "components/software/SoftwareRpmSensor.h"

    // Services
    #include "services/IService.h"
    #include "services/ServiceList.h"
    #include "services/ServiceScheduler.h"
    #include "services/SCar.h"
    #include "services/SSpeed.h"

    // Programs
    #include "programs/IProgram.h"
    #include "programs/PBrakeLight.h"
    #include "programs/PCooling.h"
    #include "programs/PMotorController.h"
    
#endif

#ifdef USE_ARDUINO

#endif