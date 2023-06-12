#ifndef MASTERCONF_H
#define MASTERCONF_H

// #define REPORT_ERRORS

// #define CAN_DEBUG // Enables CAN Service Debug Output (Log almost every step done by the
//                      CANService over Serial)

// #define MOTORCONTROLLER_OUTPUT // Output the Value sent to the MotorController over Serial

// #define FORCE_DISABLE_HV_CHECK // Disables HV-Checks (HardwareHvEnabled always returns true) !!!
//                                    ONLY USE FOR DEBUGGING WITHOUT HV-ACCU INSTALLED !!!

// #define PRINT_SPEED // Speed red in PCooling
// #define SSPEED_REPORT_SPEED
#define SSPEED_FORCED_USE_MOTOR
// #define SSPEED_REPORT_MOTOR_RPM
// #define SCAR_PRINT_POWER_SETTING
// #define PMOTORCONTROLLER_DISABLE_MOTOR_POWER_OUTPUT
#define PMOTORCONTROLLER_ACTIVATE_RECUPERATION
#define PMOTORCONTROLLER_USE_BRAKE_FOR_RECUPERATION
// #define PMOTORCONTROLLER_PRINT_CURRENTLY_MAX_CURRENT
// #define DISABLE_PUMP
// #define EXPERIMENTELL_ASR_ACTIVE
#define ENABLE_POWER_MENU
// #define REPORT_CAN_ERROR
#include "carpi.h"

#define HIGH_DEMAND_SERVICE_REFRESH_RATE 120 // Hz
#define LOW_DEMAND_SERVICE_REFRESH_RATE 3    // Hz

#include "hardware/Pins_MasterTest.h"

#ifdef EXPERIMENTELL_ASR_ACTIVE
#warning "Don't forget to activate ASR on PedalSlave too!!!"
#endif

CANService canService(MASTER_CAN_RX, MASTER_CAN_TX);

// Components
//   Software
//     Dashboard
//       LED's
SoftwareLed ledRed(COMPONENT_LED_RED);
SoftwareLed ledGreen(COMPONENT_LED_GREEN);
SoftwareLed ledBlue(COMPONENT_LED_BLUE);
SoftwareLed ledCal(COMPONENT_LED_CAL);
SoftwareLed ledRtd(COMPONENT_LED_READY_TO_DRIVE);
SoftwareLed ledLct(COMPONENT_LED_LAUNCH_CONTROL);
SoftwareLed ledImd(COMPONENT_LED_IMD);
SoftwareLed ledBms(COMPONENT_LED_BMS);

//       Buttons
SoftwareButton buttonReset(COMPONENT_BUTTON_RESET);
SoftwareButton buttonStart(COMPONENT_BUTTON_START);
SoftwareButton buttonCal(COMPONENT_BUTTON_CAL);
SoftwareButton buttonTsOn(COMPONENT_BUTTON_TS_ON);

//     Pedal
//       Pedals
SoftwarePedal gasPedal(COMPONENT_PEDAL_GAS);
SoftwarePedal brakePedal(COMPONENT_PEDAL_BRAKE);

//     Display
SoftwareDisplay display(COMPONENT_DISPLAY_MAIN);

#ifdef RPM_SENSOR
//       RPM Sensors (at Pedal Box)
SoftwareRpmSensor rpmFrontLeft(COMPONENT_RPM_FRONT_LEFT);
SoftwareRpmSensor rpmFrontRight(COMPONENT_RPM_FRONT_RIGHT);
#endif

//   Hardware
HardwareLed brakeLight(MASTER_BRAKELIGHT_OUT, COMPONENT_LED_BRAKE);
HardwareMotorController motorController(MASTER_INVERTER_CAN_RX,
                                        MASTER_INVERTER_CAN_TX, MASTER_RFE_OUT,
                                        MASTER_RUN_OUT, BAMOCAR_D3_400V,
                                        COMPONENT_MOTOR_MAIN, true);
HardwareFan coolingFan(MASTER_FANS_PWM_OUT, COMPONENT_COOLING_FAN);
HardwarePump coolingPump(MASTER_PUMP_PWM_OUT, COMPONENT_COOLING_PUMP);
HardwarePwmBuzzer buzzer(MASTER_BUZZER_OUT, COMPONENT_BUZZER_STARTUP);

// HardwareHvEnabled hvEnabled(MASTER_PIN_HV_ALL_READY, COMPONENT_SYSTEM_60V_OK);           Removed in new PCB
// HardwareHvEnabled tsms(MASTER_SHUTDOWN_TSMS_MONITORING, COMPONENT_SYSTEM_TSMS); 	        // This or Shutdown Monitoring

// INTEGRATE BETTER LATER
DigitalOut bspdTestOut(MASTER_BSPD_TEST_OUT);

// RPM Sensors

// DigitalIn x1(MASTER_PIN_RPM_RL, OpenDrain);                                    // not used
// DigitalIn x2(MASTER_PIN_RPM_RR, OpenDrain);                                    // not used

// Shutdown Monitoring

DigitalIn shutdownAfterAccu(MASTER_SHUTDOWN_ACCU_MONITORING, PullUp);
DigitalIn shutdownAfterBSPD(MASTER_SHUTDOWN_BSPD_MONITORING, PullUp);
DigitalIn shutdownAfterHvd(MASTER_SHUTDOWN_HVD_MONITORING, PullUp);
DigitalIn shutdownAfterInverter(MASTER_SHUTDOWN_INVERTER_MONITORING, PullUp);
DigitalIn shutdownAfterMainhoop(MASTER_SHUTDOWN_MAINHOOP_MONITORING, PullUp);
DigitalIn shutdownAfterTSMS(MASTER_SHUTDOWN_TSMS_MONITORING, PullUp);
SoftwareHvEnabled hvEnabled(COMPONENT_SYSTEM_60V_OK);

// Brake pressure monitoring

AnalogIn brakePressure(MASTER_BRAKE_PRESSURE_SENSOR);

// Inverter monitoring

DigitalIn inverterDin1(MASTER_DOUT1_IN);
DigitalIn inverterDin2(MASTER_DOUT2_IN);

// DigitalOut stopPrechargeOut(MASTER_PIN_STOP_PRECHARGE_OUT);                    // removed in new PCB

PBrakeLight brakeLightService((IPedal *)&brakePedal, (ILed *)&brakeLight);

SLed ledService(canService, (ILed *)&ledRed, (ILed *)&ledGreen, (ILed *)&ledBlue, (ILed *)&ledCal, (ILed *)&ledRtd, (ILed *)&ledLct);

SCar carService(canService, ledService, (IButton *)&buttonReset, (IButton *)&buttonCal, (IButton *)&buttonStart, (IButton *)&buttonTsOn, (IPedal *)&gasPedal, (IPedal *)&brakePedal,
                (IBuzzer *)&buzzer, (IMotorController *)&motorController, (IHvEnabled *)&hvEnabled,
                (DigitalIn *)&shutdownAfterTSMS, brakeLightService);

#ifdef RPM_SENSOR
SSpeed speedService(carService, (IRpmSensor *)&rpmFrontLeft, (IRpmSensor *)&rpmFrontRight,
                    /* (IRpmSensor*)&rpmRearLeft, (IRpmSensor*)&rpmRearRight, */ // [il]
                    (IMotorController *)&motorController);
#else
SSpeed speedService(carService, (IMotorController *)&motorController);
#endif

#ifdef RPM_SENSOR

PMotorController motorControllerService(carService, ledService, (IMotorController *)&motorController,
                                        (IPedal *)&gasPedal, (IPedal *)&brakePedal,
                                        (IRpmSensor *)&rpmFrontLeft, (IRpmSensor *)&rpmFrontRight,
                                        speedService);
#else
PMotorController motorControllerService(carService, ledService, (IMotorController *)&motorController,
                                        (IPedal *)&gasPedal, (IPedal *)&brakePedal,
                                        speedService);
#endif

PCooling coolingService(carService, speedService, (IFan *)&coolingFan, (IPump *)&coolingPump,
                        (IMotorController *)&motorController, (IHvEnabled *)&hvEnabled);

SDisplay displayService(canService, carService, speedService, (IMotorController *)&motorController, (IDisplay *)&display, (IPedal *)&gasPedal, (IPedal *)&brakePedal, (DigitalIn *)&shutdownAfterAccu, (DigitalIn *)&shutdownAfterBSPD, (DigitalIn *)&shutdownAfterHvd, (DigitalIn *)&shutdownAfterInverter, (DigitalIn *)&shutdownAfterMainhoop, (DigitalIn *)&shutdownAfterTSMS);

class Master : public Carpi
{
public:
    /**
     * Adds Components to Can
     */
    void addToCan()
    {
        // Display
        canService.addComponent((ICommunication *)&display);

        // Add all Software Components to the CAN Service
        // Dashboard
        canService.addComponent((ICommunication *)&ledRed);
        canService.addComponent((ICommunication *)&ledCal);
        canService.addComponent((ICommunication *)&ledRtd);
        canService.addComponent((ICommunication *)&ledLct);
        canService.addComponent((ICommunication *)&ledImd);
        canService.addComponent((ICommunication *)&ledBms);
        canService.addComponent((ICommunication *)&buttonReset);
        canService.addComponent((ICommunication *)&buttonStart);
        canService.addComponent((ICommunication *)&buttonCal);
        canService.addComponent((ICommunication *)&buttonTsOn);

        // Pedal
        canService.addComponent((ICommunication *)&gasPedal);
        canService.addComponent((ICommunication *)&brakePedal);

        // Accumulator

#ifdef EXPERIMENTELL_ASR_ACTIVE
        // RPM
        canService.addComponent((ICommunication *)&rpmFrontLeft);
        canService.addComponent((ICommunication *)&rpmFrontRight);
#endif
    }
    // Called once at bootup
    void setup()
    {
        bspdTestOut.write(0);
        wait_us(200000);

        canService.setSenderId(DEVICE_MASTER);

        addToCan();

        // Add all high demand Services to our Service list
        highDemandServices.addRunable((IRunable *)&canService);
        highDemandServices.addRunable((IRunable *)&carService);
        highDemandServices.addRunable((IRunable *)&speedService);
        highDemandServices.addRunable((IRunable *)&motorControllerService);
        highDemandServices.addRunable((IRunable *)&brakeLightService);
        highDemandServices.addRunable((IRunable *)&displayService);

        // Add all low demand Services to our Service list
        lowDemandServices.addRunable((IRunable *)&coolingService);
        lowDemandServices.addRunable((IRunable *)&ledService);

        // Add all Services and ServiceLists to our ServiceScheduler
        services.addRunable((IRunable *)&highDemandServices, HIGH_DEMAND_SERVICE_REFRESH_RATE);
        services.addRunable((IRunable *)&lowDemandServices, LOW_DEMAND_SERVICE_REFRESH_RATE);

        // Start the Car
        carService.startUp();
    }

    // Called repeately after bootup
    void loop()
    {

        // Run all Services
        services.run();

        // Send this via CAN now [il]

        // if (inverterDin1.read() == 1 && inverterDin2.read() == 1)
        // {
        //     stopPrechargeOut.write(1);
        // }
        // else
        // {
        //     stopPrechargeOut.write(0);
        // }

        // BSPD Test hotfix
        if (buttonReset.getState() == LONG_CLICKED)
        {
            bspdTestOut.write(1);
        }
        else
        {
            bspdTestOut.write(0);
        }

        wait_us(124);
    }

protected:
    RunableList highDemandServices;
    RunableList lowDemandServices;
    RunableScheduler services;
};

Master runtime;

#endif
