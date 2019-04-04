#ifndef MASTERCONF_H
#define MASTERCONF_H

//#define CAN_DEBUG
//#define MOTORCONTROLLER_OUTPUT
#include "carpi.h"

#define HIGH_DEMAND_SERVICE_REFRESH_RATE 120 // Hz
#define LOW_DEMAND_SERVICE_REFRESH_RATE 3 // Hz

#include "hardware/Pins_Master.h"

CANService canService(MASTER_PIN_CAR_INTERN_CAN_RD, MASTER_PIN_CAR_INTERN_CAN_TD);

// Components
//   Software
//     Dashboard
//       LED's
SoftwareLed ledRed(COMPONENT_LED_ERROR);
SoftwareLed ledYellow(COMPONENT_LED_ISSUE);
SoftwareLed ledGreen(COMPONENT_LED_READY_TO_DRIVE);

//       Buttons
SoftwareButton buttonReset(COMPONENT_BUTTON_RESET);
SoftwareButton buttonStart(COMPONENT_BUTTON_START);

//     Pedal
//       Pedals
SoftwarePedal gasPedal(COMPONENT_PEDAL_GAS);
SoftwarePedal brakePedal(COMPONENT_PEDAL_BRAKE);

//       RPM Sensors (at Pedal Box)
SoftwareRpmSensor rpmFrontLeft(COMPONENT_RPM_FRONT_LEFT);
SoftwareRpmSensor rpmFrontRight(COMPONENT_RPM_FRONT_RIGHT);

//   Hardware
HardwareLed brakeLight(MASTER_PIN_BRAKE_LIGHT, COMPONENT_LED_BRAKE);
HardwareMotorController motorController(MASTER_PIN_MOTOR_CONTROLLER_CAN_RD, MASTER_PIN_MOTOR_CONTROLLER_CAN_TD, MASTER_PIN_RFE_ENABLE, MASTER_PIN_RUN_ENABLE, COMPONENT_MOTOR_MAIN);
//HardwareRpmSensor rpmRearLeft(MASTER_PIN_RPM_SENSOR_HL, RPM_REAR_LEFT); // [il]
//HardwareRpmSensor rpmRearRight(MASTER_PIN_RPM_SENSOR_HR, RPM_REAR_RIGHT); // [il]
HardwareFan coolingFan(MASTER_PIN_FAN, COMPONENT_COOLING_FAN);
HardwarePump coolingPump(MASTER_PIN_PUMP_PWM, MASTER_PIN_PUMP_ENABLE, COMPONENT_COOLING_PUMP);
HardwareBuzzer buzzer(MASTER_PIN_BUZZER, COMPONENT_BUZZER_STARTUP);
DigitalIn hvEnabled(MASTER_PIN_HV_ENABLED); // [QF]

// Services
SCar carService(canService,
                (IButton*)&buttonReset, (IButton*)&buttonStart,
                (ILed*)&ledRed, (ILed*)&ledYellow, (ILed*)&ledGreen,
                (IPedal*)&gasPedal, (IPedal*)&brakePedal,
                (IBuzzer*)&buzzer,
                (IMotorController*)&motorController,
                hvEnabled);

PMotorController motorControllerService(carService,
                                        (IMotorController*)&motorController,
                                        (IPedal*)&gasPedal, (IPedal*)&brakePedal);

PBrakeLight brakeLightService(carService, (IPedal*)&brakePedal, (ILed*)&brakeLight);

SSpeed speedService(carService,
                    (IRpmSensor*)&rpmFrontLeft, (IRpmSensor*)&rpmFrontRight, /* (IRpmSensor*)&rpmRearLeft, (IRpmSensor*)&rpmRearRight, */ // [il]
                    (IMotorController*)&motorController);

PCooling coolingService(carService,
                        speedService,
                        (IFan*)&coolingFan, (IPump*)&coolingPump,
                        (IMotorController*)&motorController,
                        hvEnabled);

class Master {
    public:
        // Called once at bootup
        void setup() {
            wait(2);

            canService.setSenderId(DEVICE_MASTER);

            // Add all Software Components to the CAN Service
            // Dashboard
            canService.addComponent((ICommunication*)&ledRed);
            canService.addComponent((ICommunication*)&ledYellow);
            canService.addComponent((ICommunication*)&ledGreen);
            canService.addComponent((ICommunication*)&buttonReset);
            canService.addComponent((ICommunication*)&buttonStart);

            // Pedal
            canService.addComponent((ICommunication*)&gasPedal);
            canService.addComponent((ICommunication*)&brakePedal);
            canService.addComponent((ICommunication*)&rpmFrontLeft);
            canService.addComponent((ICommunication*)&rpmFrontRight);



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
            wait(0.001);
        }
    
    protected:
        ServiceList highDemandServices;
        ServiceList lowDemandServices;
        ServiceScheduler services;
};

Master runtime;

#endif