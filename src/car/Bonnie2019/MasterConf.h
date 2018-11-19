#ifndef BONNIE_2019_MASTER_CONF_H
#define BONNIE_2019_MASTER_CONF_H

#include "mbed.h"
#include "hardware/Pins_Master.h"

#define HIGH_DEMAND_SERVICE_REFRESH_RATE 120 // Hz
#define LOW_DEMAND_SERVICE_REFRESH_RATE 15 // Hz

// Include for Dashboard Components
#include "../../components/software/SoftwareLed.h"
#include "../../can/LEDMessageHandler.h"
#include "../../components/software/SoftwareButton.h"
#include "../../can/ButtonMessageHandler.h"

// Include for Pedal Components
#include "../../components/software/SoftwarePedal.h"
#include "../../can/PedalMessageHandler.h"

// Include for Hardware
#include "../../components/hardware/HardwareLed.h"
#include "../../components/hardware/HardwareMotorController.h"

// Include for Services
#include "../../components/service/CarService.h"
#include "../../components/service/MotorControllerService.h"
#include "../../components/service/BrakeLightService.h"
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

//   Hardware
HardwareLed brakeLight(MASTER_PIN_BRAKE_LIGHT); // change pin
HardwareMotorController motorController(CAN2_CONF);

// Services
CarService carService((IButton*)&buttonReset, (IButton*)&buttonStart,
                      (ILed*)&ledRed, (ILed*)&ledYellow, (ILed*)&ledGreen,
                      (IPedal*)&gasPedal, (IPedal*)&brakePedal);

MotorControllerService motorControllerService(carService,
                                              (IMotorController*)&motorController,
                                              (IPedal*)&gasPedal, (IPedal*)&brakePedal);

BrakeLightService brakeLightService(carService, (IPedal*)&brakePedal, (ILed*)&brakeLight);

class Master {
    public:
        // Called once at bootup
        void setup() {
            // Add all high demand Services to our Service list
            highDemandServices.addService((IService*)&canService);
            highDemandServices.addService((IService*)&carService);
            highDemandServices.addService((IService*)&motorControllerService);
            highDemandServices.addService((IService*)&brakeLightService);

            // Add all Services and ServiceLists to our ServiceScheduler
            services.addService((IService*)&highDemandServices, HIGH_DEMAND_SERVICE_REFRESH_RATE);

            // Start the Car
            carService.startUp();
        }

        // Called repeately after bootup
        void loop() {
            services.run();
        }
    
    protected:
        ServiceList highDemandServices;
        //ServiceList lowDemandServices;
        ServiceScheduler services;
};

Master runtime;

#endif