#ifndef MASTERCONF_H
#define MASTERCONF_H

//#define CAN_DEBUG // Enables CAN Service Debug Output (Log almost every step done by the CANService over Serial)
//#define MOTORCONTROLLER_OUTPUT // Output the Value sent to the MotorController over Serial
//#define FORCE_DISABLE_HV_CHECK // Disables HV-Checks (HardwareHvEnabled always returns true) !!! ONLY USE FOR DEBUGGING WITHOUT HV-ACCU INSTALLED !!!
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
SoftwareLed ledCI(COMPONENT_LED_CI);

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
HardwareHvEnabled hvEnabled(MASTER_PIN_HV_ENABLED, COMPONENT_SYSTEM_HV_ENABLED);

// Services
SCar carService(canService,
                (IButton*)&buttonReset, (IButton*)&buttonStart,
                (ILed*)&ledRed, (ILed*)&ledYellow, (ILed*)&ledGreen,
                (IPedal*)&gasPedal, (IPedal*)&brakePedal,
                (IBuzzer*)&buzzer,
                (IMotorController*)&motorController,
                (IHvEnabled*)&hvEnabled);

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
                        (IHvEnabled*)&hvEnabled);

PCockpitIndicator cockpitIndicatorProgram(canService, hvEnabled, ledCI);

class Master : public Carpi {
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
            canService.addComponent((ICommunication*)&ledCI);
            canService.addComponent((ICommunication*)&buttonReset);
            canService.addComponent((ICommunication*)&buttonStart);

            // Pedal
            canService.addComponent((ICommunication*)&gasPedal);
            canService.addComponent((ICommunication*)&brakePedal);
            canService.addComponent((ICommunication*)&rpmFrontLeft);
            canService.addComponent((ICommunication*)&rpmFrontRight);



            // Add all high demand Services to our Service list
            highDemandServices.addRunable((IRunable*)&canService);
            highDemandServices.addRunable((IRunable*)&carService);
            highDemandServices.addRunable((IRunable*)&motorControllerService);
            highDemandServices.addRunable((IRunable*)&brakeLightService);

            // Add all low demand Services to our Service list
            lowDemandServices.addRunable((IRunable*)&speedService);
            lowDemandServices.addRunable((IRunable*)&coolingService);
            lowDemandServices.addRunable((IRunable*)&cockpitIndicatorProgram);

            // Add all Services and ServiceLists to our ServiceScheduler
            services.addRunable((IRunable*)&highDemandServices, HIGH_DEMAND_SERVICE_REFRESH_RATE);
            services.addRunable((IRunable*)&lowDemandServices, LOW_DEMAND_SERVICE_REFRESH_RATE);

            // Start the Car
            carService.startUp();
        }

        // Called repeately after bootup
        void loop() {
            services.run();
            wait(0.001);
        }
    
    protected:
        RunableList highDemandServices;
        RunableList lowDemandServices;
        RunableScheduler services;
};

Master runtime;

#endif