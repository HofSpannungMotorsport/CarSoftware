#ifndef MASTERCONF_H
#define MASTERCONF_H

//#define REPORT_ERRORS

//#define CAN_DEBUG // Enables CAN Service Debug Output (Log almost every step done by the
//                     CANService over Serial)

//#define MOTORCONTROLLER_OUTPUT // Output the Value sent to the MotorController over Serial

//#define FORCE_DISABLE_HV_CHECK // Disables HV-Checks (HardwareHvEnabled always returns true) !!!
//                                  ONLY USE FOR DEBUGGING WITHOUT HV-ACCU INSTALLED !!!

//#define PRINT_SPEED // Speed red in PCooling
//#define SSPEED_REPORT_SPEED
#define SSPEED_FORCED_USE_MOTOR
//#define SSPEED_REPORT_MOTOR_RPM
//#define SCAR_PRINT_POWER_SETTING
//#define PMOTORCONTROLLER_DISABLE_MOTOR_POWER_OUTPUT
#define PMOTORCONTROLLER_ACTIVATE_RECUPERATION
#define PMOTORCONTROLLER_USE_BRAKE_FOR_RECUPERATION
//#define PMOTORCONTROLLER_PRINT_CURRENTLY_MAX_CURRENT
//#define DISABLE_PUMP
//#define EXPERIMENTELL_ASR_ACTIVE
//#define ENABLE_POWER_MENU
//#define REPORT_CAN_ERROR
#include "carpi.h"

#define HIGH_DEMAND_SERVICE_REFRESH_RATE 120  // Hz
#define LOW_DEMAND_SERVICE_REFRESH_RATE 3     // Hz

#include "hardware/Pins_Master.h"

#ifdef EXPERIMENTELL_ASR_ACTIVE
#warning "Don't forget to activate ASR on PedalSlave too!!!"
#endif

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
SoftwareButton buttonCal(COMPONENT_BUTTON_CAL);

//     Pedal
//       Pedals
SoftwarePedal gasPedal(COMPONENT_PEDAL_GAS);
SoftwarePedal brakePedal(COMPONENT_PEDAL_BRAKE);

//       RPM Sensors (at Pedal Box)
SoftwareRpmSensor rpmFrontLeft(COMPONENT_RPM_FRONT_LEFT);
SoftwareRpmSensor rpmFrontRight(COMPONENT_RPM_FRONT_RIGHT);

//   Hardware
HardwareLed brakeLight(MASTER_PIN_BRAKE_LIGHT, COMPONENT_LED_BRAKE);
HardwareMotorController motorController(MASTER_PIN_MOTOR_CONTROLLER_CAN_RD,
                                        MASTER_PIN_MOTOR_CONTROLLER_CAN_TD, MASTER_PIN_RFE_ENABLE,
                                        MASTER_PIN_RUN_ENABLE, BAMOCAR_D3_400V,
                                        COMPONENT_MOTOR_MAIN, true);
HardwareFan coolingFan(MASTER_PIN_FAN, COMPONENT_COOLING_FAN);
HardwarePump coolingPump(MASTER_PIN_PUMP_PWM, COMPONENT_COOLING_PUMP);
HardwareBuzzer buzzer(MASTER_PIN_BUZZER, COMPONENT_BUZZER_STARTUP);
HardwareHvEnabled hvEnabled(MASTER_PIN_HV_ALL_READY, COMPONENT_SYSTEM_60V_OK);
HardwareHvEnabled tsms(MASTER_PIN_TSMS, COMPONENT_SYSTEM_TSMS);

// INTEGRATE BETTER LATER
DigitalOut bspdTestOut(MASTER_PIN_BSPD_TEST);

DigitalIn x1(MASTER_PIN_RPM_RL, OpenDrain);
DigitalIn x2(MASTER_PIN_RPM_RR, OpenDrain);
DigitalIn x3(MASTER_PIN_SHUTDOWN_PRE_BSPD, OpenDrain);
DigitalIn x4(MASTER_PIN_SHUTDOWN_AFTER_BSPD, OpenDrain);
DigitalIn x5(MASTER_PIN_SHUTDOWN_AT_TS_ON, OpenDrain);
DigitalIn x7(MASTER_PIN_SHUTDOWN_AT_BOTS, OpenDrain);
DigitalIn x8(MASTER_PIN_SHUTDOWN_ERROR_STORAGE, OpenDrain);
DigitalIn x9(MASTER_PIN_SHUTDOWN_TSMS_IN, OpenDrain);
DigitalIn x10(MASTER_PIN_IMD_OK, OpenDrain);
DigitalIn x11(MASTER_PIN_BMS_OK, OpenDrain);
DigitalIn x12(MASTER_PIN_TS_ON_STATE, OpenDrain);
DigitalIn x13(MASTER_PIN_TSAL_MC_OUT, OpenDrain);
DigitalIn x14(MASTER_PIN_BRAKE_FRONT, OpenDrain);

DigitalIn inverterDin1(MASTER_PIN_INVERTER_DOUT_1);
DigitalIn inverterDin2(MASTER_PIN_INVERTER_DOUT_2);
DigitalOut stopPrechargeOut(MASTER_PIN_STOP_PRECHARGE_OUT);

PBrakeLight brakeLightService((IPedal*)&brakePedal, (ILed*)&brakeLight);

SCar carService(canService, (IButton*)&buttonReset, (IButton*)&buttonStart, (ILed*)&ledRed,
                (ILed*)&ledYellow, (ILed*)&ledGreen, (IPedal*)&gasPedal, (IPedal*)&brakePedal,
                (IBuzzer*)&buzzer, (IMotorController*)&motorController, (IHvEnabled*)&hvEnabled,
                (IHvEnabled*)&tsms, brakeLightService);

SSpeed speedService(carService, (IRpmSensor*)&rpmFrontLeft, (IRpmSensor*)&rpmFrontRight,
                    /* (IRpmSensor*)&rpmRearLeft, (IRpmSensor*)&rpmRearRight, */  // [il]
                    (IMotorController*)&motorController);

PMotorController motorControllerService(carService, (IMotorController*)&motorController,
                                        (IPedal*)&gasPedal, (IPedal*)&brakePedal,
                                        (IRpmSensor*)&rpmFrontLeft, (IRpmSensor*)&rpmFrontRight,
                                        speedService);

PCooling coolingService(carService, speedService, (IFan*)&coolingFan, (IPump*)&coolingPump,
                        (IMotorController*)&motorController, (IHvEnabled*)&hvEnabled);

class Master : public Carpi {
   public:
    // Called once at bootup
    void setup() {
        bspdTestOut.write(0);

        wait(2);

        canService.setSenderId(DEVICE_MASTER);

        // Add all Software Components to the CAN Service
        // Dashboard
        canService.addComponent((ICommunication*)&ledRed);
        canService.addComponent((ICommunication*)&ledYellow);
        canService.addComponent((ICommunication*)&ledGreen);
        canService.addComponent((ICommunication*)&buttonReset);
        canService.addComponent((ICommunication*)&buttonStart);
        canService.addComponent((ICommunication*)&buttonCal);

        // Pedal
        canService.addComponent((ICommunication*)&gasPedal);
        canService.addComponent((ICommunication*)&brakePedal);

        // RPM
        canService.addComponent((ICommunication*)&rpmFrontLeft);
        canService.addComponent((ICommunication*)&rpmFrontRight);

        #ifdef EXPERIMENTAL_DISPLAY_ACTIVE
        // Motor
        canService.addComponent((ICommunication*)&motorController);
        canService.addComponentToSendLoop((ICommunication*)&motorController);
        #endif 


        // Add all high demand Services to our Service list
        highDemandServices.addRunable((IRunable*)&canService);
        highDemandServices.addRunable((IRunable*)&carService);
        highDemandServices.addRunable((IRunable*)&speedService);
        highDemandServices.addRunable((IRunable*)&motorControllerService);
        highDemandServices.addRunable((IRunable*)&brakeLightService);

        // Add all low demand Services to our Service list
        lowDemandServices.addRunable((IRunable*)&coolingService);

        // Add all Services and ServiceLists to our ServiceScheduler
        services.addRunable((IRunable*)&highDemandServices, HIGH_DEMAND_SERVICE_REFRESH_RATE);
        services.addRunable((IRunable*)&lowDemandServices, LOW_DEMAND_SERVICE_REFRESH_RATE);

        // Start the Car
        // microcontrollerOk = 1;
        carService.startUp();
    }

    // Called repeately after bootup
    void loop() {
        services.run();

        if (inverterDin1.read() == 1 && inverterDin2.read() == 1) {
            stopPrechargeOut.write(1);
        } else {
            stopPrechargeOut.write(0);
        }

        // BSPD Test hotfix
        if (buttonCal.getState() == LONG_CLICKED) {
            bspdTestOut.write(1);
        } else {
            bspdTestOut.write(0);
        }

        wait(0.001);
    }

   protected:
    RunableList highDemandServices;
    RunableList lowDemandServices;
    RunableScheduler services;
};

Master runtime;

#endif
