#ifndef MASTERCONF_H
#define MASTERCONF_H

//#define MOTORCONTROLLER_OUTPUT // Output the Value sent to the MotorController over Serial
//#define FORCE_DISABLE_HV_CHECK // Disables HV-Checks (HardwareHvEnabled always returns true) !!! ONLY USE FOR DEBUGGING WITHOUT HV-ACCU INSTALLED !!!
//#define SYNC_DEBUG // Enables reporting for Sync
//#define SYNC_SENDING_DEBUG // Enables reporting while sending a message over Sync
//#define CCAN_DEBUG // Enables CAN Channel Debug Output
//#define CCAN_SENDING_DEBUG // Enables reporting at sending over Can
//#define DISABLE_PUMP // Disables the Pump entirly, usefull if the cooling system is not filled with water !!! DISCONNECT PUMP ON UPLOAD IF NOT FILLED WITH WATER, OTHERWISE IT WILL START !!!
//#define DISABLE_SERIAL // Disables the serial report funktion
//#define PRINT_SPEED
#include "carpi.h"

#define HIGH_DEMAND_SERVICE_REFRESH_RATE 120 // Hz
#define LOW_DEMAND_SERVICE_REFRESH_RATE 3 // Hz

#ifdef MASTER_SMALL
    #include "hardware/Pins_Master_PCB_Adapter.h"
#else
    #include "hardware/Pins_Master_PCB.h"
#endif

#include "SDLog.h"

// Communication
Sync syncer(DEVICE_MASTER);
CCan canIntern(syncer, MASTER_PIN_CAR_INTERN_CAN_RD, MASTER_PIN_CAR_INTERN_CAN_TD);

// Components
//   Internal
//     Registry
InternalRegistry registry(COMPONENT_SYSTEM_REGISTRY);

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

//       Alive
SoftwareAlive dashboardAlive(COMPONENT_ALIVE_DASHBOARD, registry);

//     Pedal
//       Pedals
SoftwarePedal gasPedal(COMPONENT_PEDAL_GAS);
SoftwarePedal brakePedal(COMPONENT_PEDAL_BRAKE);

//       RPM Sensors (at Pedal Box)
//SoftwareRpmSensor rpmFrontLeft(COMPONENT_RPM_FRONT_LEFT);
//SoftwareRpmSensor rpmFrontRight(COMPONENT_RPM_FRONT_RIGHT);

//       Alive
SoftwareAlive pedalAlive(COMPONENT_ALIVE_PEDAL, registry);

//   Hardware
HardwareLed brakeLight(MASTER_PIN_BRAKE_LIGHT, COMPONENT_LED_BRAKE, registry);
HardwareMotorController motorController(MASTER_PIN_MOTOR_CONTROLLER_CAN_RD, MASTER_PIN_MOTOR_CONTROLLER_CAN_TD, MASTER_PIN_RFE_ENABLE, MASTER_PIN_RUN_ENABLE, COMPONENT_MOTOR_MAIN, registry);
//HardwareRpmSensor rpmRearLeft(MASTER_PIN_RPM_SENSOR_HL, RPM_REAR_LEFT); // [il]
//HardwareRpmSensor rpmRearRight(MASTER_PIN_RPM_SENSOR_HR, RPM_REAR_RIGHT); // [il]
HardwareFan coolingFan(MASTER_PIN_FAN, COMPONENT_COOLING_FAN);
HardwarePump coolingPump(MASTER_PIN_PUMP_PWM, MASTER_PIN_PUMP_ENABLE, COMPONENT_COOLING_PUMP);
HardwareBuzzer buzzer(MASTER_PIN_BUZZER, COMPONENT_BUZZER_STARTUP, registry);
HardwareHvEnabled hvEnabled(MASTER_PIN_60V_OK, COMPONENT_SYSTEM_60V_OK, HV_ENABLED_ON_AT_LOW);
HardwareHvEnabled tsms(MASTER_PIN_TSMS, COMPONENT_SYSTEM_TSMS);
HardwareSDCard hardwareSD(COMPONENT_SYSTEM_SD_CARD, MASTER_PIN_SPI_SD_MOSI, MASTER_PIN_SPI_SD_MISO, MASTER_PIN_SPI_SD_SCK, MASTER_PIN_SPI_SD_CS);
HardwareAlive masterAlive(COMPONENT_ALIVE_MASTER, MASTER_PIN_MICROCONTROLLER_OK, registry);

DigitalOut microcontrollerOk(MASTER_PIN_MICROCONTROLLER_OK);

// Services
PCockpitIndicator cockpitIndicatorProgram(hvEnabled, ledCI);

PBrakeLight brakeLightService(brakePedal, brakeLight, registry);

SCar carService(syncer,
                registry,
                buttonReset, buttonStart,
                ledRed, ledYellow, ledGreen,
                gasPedal, brakePedal,
                buzzer,
                motorController,
                hvEnabled,
                tsms,
                hardwareSD,
                pedalAlive, dashboardAlive, masterAlive,
                cockpitIndicatorProgram,
                brakeLightService);

PMotorController motorControllerService(carService, registry,
                                        motorController,
                                        gasPedal, brakePedal);

SSpeed speedService(carService,
                    /*rpmFrontLeft, rpmFrontRight, rpmRearLeft, rpmRearRight, */ // [il]
                    motorController, registry);

PCooling coolingService(carService,
                        speedService,
                        coolingFan, coolingPump,
                        motorController,
                        hvEnabled,
                        registry);

PLogger sdLogger(carService, hardwareSD);

class Master : public Carpi {
    public:
        // Called once at bootup
        void setup() {
            // Only add Registry to sync Values before starting the actual program
            syncer.addComponent(registry, canIntern, DEVICE_ALL);
            registry.attach(syncer);

            // Load Hard-Coded Registry and Sync while sending to avoid buffer overflow
            InternalRegistryHardStorage::loadIn(registry, &syncer);

            // Add all Software Components to the Syncer
            // Dashboard
            syncer.addComponent(ledRed, canIntern, DEVICE_DASHBOARD);
            syncer.addComponent(ledYellow, canIntern, DEVICE_DASHBOARD);
            syncer.addComponent(ledGreen, canIntern, DEVICE_DASHBOARD);
            syncer.addComponent(ledCI, canIntern, DEVICE_DASHBOARD);
            syncer.addComponent(buttonReset, canIntern, DEVICE_DASHBOARD);
            syncer.addComponent(buttonStart, canIntern, DEVICE_DASHBOARD);
            syncer.addComponent(dashboardAlive, canIntern, DEVICE_DASHBOARD);

            // Pedal
            syncer.addComponent(gasPedal, canIntern, DEVICE_PEDAL);
            syncer.addComponent(brakePedal, canIntern, DEVICE_PEDAL);
            //syncer.addComponent(rpmFrontLeft, canIntern, DEVICE_PEDAL);
            //syncer.addComponent(rpmFrontRight, canIntern, DEVICE_PEDAL);
            syncer.addComponent(pedalAlive, canIntern, DEVICE_PEDAL);

            syncer.finalize();


            // Add all high demand Services to our Service list
            highDemandServices.addRunable(carService);
            highDemandServices.addRunable(motorControllerService);
            highDemandServices.addRunable(brakeLightService);

            // Add all low demand Services to our Service list
            lowDemandServices.addRunable(speedService);
            lowDemandServices.addRunable(coolingService);
            lowDemandServices.addRunable(cockpitIndicatorProgram);

            // Add all Services and ServiceLists to the ServiceScheduler
            serviceScheduler.addRunable(highDemandServices, HIGH_DEMAND_SERVICE_REFRESH_RATE);
            serviceScheduler.addRunable(lowDemandServices, LOW_DEMAND_SERVICE_REFRESH_RATE);

            // And then in the final service list
            services.addRunable(serviceScheduler);
            services.addRunable(sdLogger);
            services.addRunable(syncer);

            // After adding all, optimise them for ram
            highDemandServices.finalize();
            lowDemandServices.finalize();
            serviceScheduler.finalize();
            services.finalize();


            wait(STARTUP_WAIT_TIME_MASTER);


            // Attach the Syncer to all components
            // Dashboard
            ledRed.attach(syncer);
            ledYellow.attach(syncer);
            ledGreen.attach(syncer);
            buttonReset.attach(syncer);
            buttonStart.attach(syncer);
            dashboardAlive.attach(syncer);

            // Pedal
            gasPedal.attach(syncer);
            brakePedal.attach(syncer);
            //rpmFrontLeft.attach(syncer);
            //rpmFrontRight.attach(syncer);
            pedalAlive.attach(syncer);


            // Add components to logger
            sdLogger.addLogableValue(brakePedal, SD_LOG_ID_PEDAL_POSITION, SD_LOG_REFRESH_RATE_PEDAL_POSITION);
            sdLogger.addLogableValue(gasPedal, SD_LOG_ID_PEDAL_POSITION, SD_LOG_REFRESH_RATE_PEDAL_POSITION);

            sdLogger.addLogableValue(motorController, SD_LOG_ID_MOTOR_CONTROLLER_SPEED, SD_LOG_REFRESH_RATE_MOTOR_CONTROLLER_SPEED);
            sdLogger.addLogableValue(motorController, SD_LOG_ID_MOTOR_CONTROLLER_CURRENT, SD_LOG_REFRESH_RATE_MOTOR_CONTROLLER_CURRENT);
            sdLogger.addLogableValue(motorController, SD_LOG_ID_MOTOR_CONTROLLER_MOTOR_TEMP, SD_LOG_REFRESH_RATE_MOTOR_CONTROLLER_MOTOR_TEMP);
            sdLogger.addLogableValue(motorController, SD_LOG_ID_MOTOR_CONTROLLER_SERVO_TEMP, SD_LOG_REFRESH_RATE_MOTOR_CONTROLLER_SERVO_TEMP);
            sdLogger.addLogableValue(motorController, SD_LOG_ID_MOTOR_CONTROLLER_AIR_TEMP, SD_LOG_REFRESH_RATE_MOTOR_CONTROLLER_AIR_TEMP);

            //sdLogger.addLogableValue(rpmFrontLeft, SD_LOG_ID_RPM, SD_LOG_REFRESH_RATE_RPM);
            //sdLogger.addLogableValue(rpmFrontRight, SD_LOG_ID_RPM, SD_LOG_REFRESH_RATE_RPM);
            //sdLogger.addLogableValue(rpmRearLeft, SD_LOG_ID_RPM, SD_LOG_REFRESH_RATE_RPM);
            //sdLogger.addLogableValue(rpmRearRight, SD_LOG_ID_RPM, SD_LOG_REFRESH_RATE_RPM);

            sdLogger.addLogableValue(coolingPump, SD_LOG_ID_PUMP_SPEED, SD_LOG_REFRESH_RATE_COOLING_PUMP_SPEED);

            sdLogger.finalize();
            hardwareSD.begin();
            hardwareSD.open();
            sdLogger.begin();


            // Start the Car
            microcontrollerOk = 1;
            carService.startUp();
        }

        // Called repeately after bootup
        void loop() {
            services.run();
            wait(LOOP_WAIT_TIME);
        }
    
    protected:
        RunableList highDemandServices;
        RunableList lowDemandServices;
        RunableScheduler serviceScheduler;
        RunableList services;
};

Master runtime;

#endif