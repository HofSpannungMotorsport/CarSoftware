#ifndef MASTERCONF_H
#define MASTERCONF_H

#include "mbed.h"

#ifndef MESSAGE_REPORT
    #define MESSAGE_REPORT
    Serial pcSerial(USBTX, USBRX); // Connection to PC over Serial
#endif

#include "../../can/can_config.h"
#include "../../can/CANService.h"
CANService canService(CAN1_CONF);

#include "hardware/Pins_Master.h"

#define HIGH_DEMAND_SERVICE_REFRESH_RATE 120 // Hz
#define LOW_DEMAND_SERVICE_REFRESH_RATE 3 // Hz

// Include for Dashboard Components
#include "../../components/software/SoftwareLed.h"
#include "../../can/LEDMessageHandler.h"
#include "../../components/software/SoftwareButton.h"
#include "../../can/ButtonMessageHandler.h"
#include "../../components/software/SoftwareRpmSensor.h"
#include "../../can/RpmSensorMessageHandler.h"

// Include for Pedal Components
#include "../../components/software/SoftwarePedal.h"
#include "../../can/PedalMessageHandler.h"

// Include for Hardware
#include "../../components/hardware/HardwareLed.h"
#include "../../components/hardware/HardwareMotorController.h"
#include "../../components/hardware/HardwareRpmSensor.h"
#include "../../components/hardware/HardwareFan.h"
#include "../../components/hardware/HardwarePump.h"
#include "../../components/hardware/HardwareBuzzer.h"

// Include for Services
#include "../../components/service/CarService.h"
#include "../../components/service/MotorControllerService.h"
#include "../../components/service/BrakeLightService.h"
#include "../../components/service/SpeedService.h"
#include "../../components/service/CoolingService.h"
#include "../../components/service/ServiceList.h"
#include "../../components/service/ServiceScheduler.h"

// Components
//   Software
//     Dashboard
//       LED's
SoftwareLed ledRed(LED_ERROR);
SoftwareLed ledYellow(LED_ISSUE);
SoftwareLed ledGreen(LED_READY_TO_DRIVE);
LEDMessageHandler ledMessageHandler;

//       Buttons
SoftwareButton buttonReset(BUTTON_RESET);
SoftwareButton buttonStart(BUTTON_START);
ButtonMessageHandler buttonMessageHandler;

//     Pedal
//       Pedals
SoftwarePedal gasPedal(PEDAL_GAS);
SoftwarePedal brakePedal(PEDAL_BRAKE);
PedalMessageHandler pedalMessageHandler;

//       RPM Sensors (at Pedal Box)
SoftwareRpmSensor rpmFrontLeft(RPM_FRONT_LEFT);
SoftwareRpmSensor rpmFrontRight(RPM_FRONT_RIGHT);
RpmSensorMessageHandler rpmSensorMessageHandler;

//   Hardware
HardwareLed brakeLight(MASTER_PIN_BRAKE_LIGHT, LED_BRAKE);
HardwareMotorController motorController(MASTER_PIN_MOTOR_CONTROLLER_CAN_RD, MASTER_PIN_MOTOR_CONTROLLER_CAN_TD, MASTER_PIN_RFE_ENABLE, MASTER_PIN_RUN_ENABLE, MOTOR_MAIN);
//HardwareRpmSensor rpmRearLeft(MASTER_PIN_RPM_SENSOR_HL, RPM_REAR_LEFT); // [il]
//HardwareRpmSensor rpmRearRight(MASTER_PIN_RPM_SENSOR_HR, RPM_REAR_RIGHT); // [il]
HardwareFan coolingFan(MASTER_PIN_FAN, COOLING_FAN);
HardwarePump coolingPump(MASTER_PIN_PUMP_PWM, COOLING_PUMP);
HardwareBuzzer buzzer(MASTER_PIN_BUZZER, BUZZER_ALARM);
DigitalIn hvEnabled(MASTER_PIN_HV_ENABLED);

// Services
CarService carService((IButton*)&buttonReset, (IButton*)&buttonStart,
                      (ILed*)&ledRed, (ILed*)&ledYellow, (ILed*)&ledGreen,
                      (IPedal*)&gasPedal, (IPedal*)&brakePedal,
                      (IBuzzer*)&buzzer,
                      (IMotorController*)&motorController,
                      hvEnabled);

MotorControllerService motorControllerService(carService,
                                              (IMotorController*)&motorController,
                                              (IPedal*)&gasPedal, (IPedal*)&brakePedal);

BrakeLightService brakeLightService(carService, (IPedal*)&brakePedal, (ILed*)&brakeLight);

SpeedService speedService(carService,
                          (IRpmSensor*)&rpmFrontLeft, (IRpmSensor*)&rpmFrontRight, /* (IRpmSensor*)&rpmRearLeft, (IRpmSensor*)&rpmRearRight, */
                          (IMotorController*)&motorController);

CoolingService coolingService(carService,
                              speedService,
                              (IFan*)&coolingFan, (IPump*)&coolingPump,
                              MASTER_PIN_HV_ENABLED);

class Master {
    public:
        // Called once at bootup
        void setup() {
            // Add all Software Components to the CAN Service
            // Dashboard
            canService.addComponent((void*)&ledRed, (IMessageHandler<CANMessage>*)&ledMessageHandler, NORMAL);
            canService.addComponent((void*)&ledYellow, (IMessageHandler<CANMessage>*)&ledMessageHandler, NORMAL);
            canService.addComponent((void*)&ledGreen, (IMessageHandler<CANMessage>*)&ledMessageHandler, NORMAL);
            canService.addComponent((void*)&buttonReset, (IMessageHandler<CANMessage>*)&buttonMessageHandler, NORMAL);
            canService.addComponent((void*)&buttonStart, (IMessageHandler<CANMessage>*)&buttonMessageHandler, NORMAL);

            // Pedal
            canService.addComponent((void*)&gasPedal, (IMessageHandler<CANMessage>*)&pedalMessageHandler, NORMAL);
            canService.addComponent((void*)&brakePedal, (IMessageHandler<CANMessage>*)&pedalMessageHandler, NORMAL);
            canService.addComponent((void*)&rpmFrontLeft, (IMessageHandler<CANMessage>*)&rpmSensorMessageHandler, NORMAL);
            canService.addComponent((void*)&rpmFrontRight, (IMessageHandler<CANMessage>*)&rpmSensorMessageHandler, NORMAL);



            // Add all high demand Services to our Service list
            highDemandServices.addService((IService*)&canService);
            highDemandServices.addService((IService*)&carService);
            highDemandServices.addService((IService*)&motorControllerService);
            highDemandServices.addService((IService*)&brakeLightService);

            // Add all low demand Services to our Service list
            lowDemandServices.addService((IService*)&speedService);
            lowDemandServices.addService((IService*)&coolingService);

            // Add all Services and ServiceLists to our ServiceScheduler
            services.addService((IService*)&highDemandServices, HIGH_DEMAND_SERVICE_REFRESH_RATE);
            services.addService((IService*)&lowDemandServices, LOW_DEMAND_SERVICE_REFRESH_RATE);

            // Start the Car
            carService.startUp();
        }

        // Called repeately after bootup
        void loop() {
            services.run();
        }
    
    protected:
        ServiceList highDemandServices;
        ServiceList lowDemandServices;
        ServiceScheduler services;
};

Master runtime;

#endif