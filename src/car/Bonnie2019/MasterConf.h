#ifndef BONNIE_2019_MASTER_CONF_H
#define BONNIE_2019_MASTER_CONF_H

#include "mbed.h"
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
HardwareMotorController motorController(CAN2_CONF, MOTOR_MAIN);
HardwareRpmSensor rpmRearLeft(MASTER_PIN_RPM_SENSOR_HL, RPM_REAR_LEFT);
HardwareRpmSensor rpmRearRight(MASTER_PIN_RPM_SENSOR_HR, RPM_REAR_RIGHT);
HardwareFan coolingFan(MASTER_PIN_FAN, COOLING_FAN);
HardwarePump coolingPump(MASTER_PIN_PUMP_PWM, COOLING_PUMP);

// Services
CarService carService((IButton*)&buttonReset, (IButton*)&buttonStart,
                      (ILed*)&ledRed, (ILed*)&ledYellow, (ILed*)&ledGreen,
                      (IPedal*)&gasPedal, (IPedal*)&brakePedal);

MotorControllerService motorControllerService(carService,
                                              (IMotorController*)&motorController,
                                              (IPedal*)&gasPedal, (IPedal*)&brakePedal);

BrakeLightService brakeLightService(carService, (IPedal*)&brakePedal, (ILed*)&brakeLight);

SpeedService speedService(carService,
                          (IRpmSensor*)&rpmFrontLeft, (IRpmSensor*)&rpmFrontRight, (IRpmSensor*)&rpmRearLeft, (IRpmSensor*)&rpmRearRight
                          (IMotorController*)&motorController);

CoolingService coolingService(carService,
                              speedService,
                              (IFan*)&coolingFan, (IPump*)&coolingPump,
                              MASTER_PIN_HV_ENABLED);

class Master {
    public:
        // Called once at bootup
        void setup() {
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