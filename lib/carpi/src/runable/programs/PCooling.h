#ifndef PCOOLING_H
#define PCOOLING_H

#include "IProgram.h"
#include "communication/componentIds.h"
#include "services/SCar.h"
#include "services/SSpeed.h"
#include "components/interface/IFan.h"
#include "components/interface/IPump.h"
#include "components/interface/IMotorController.h"


#define STD_COOLING_FAN_OFF_UNTIL_SPEED 7 // kM/h
#define STD_COOLING_FAN_ON_UNTIL_SPEED 5 // kM/h

// Temperatures at which the Pump will start or reach its maximum
#define STD_MOTOR_TEMP_START_PUMP   40 // °C
#define STD_MOTOR_TEMP_FULL_PUMP    60 // °C
#define STD_CONTROLLER_TEMP_START_PUMP  35 // °C
#define STD_CONTROLLER_TEMP_FULL_PUMP    40 // °C

// Absolut Max temperatures which will stop the car to prevent overheating
#define STD_MAX_MOTOR_TEMP 110 // °C (120 in datasheet, but we don't want to stress the limit)
#define STD_MAX_CONTROLLER_TEMP 65 // °C

enum cooling_service_error_type_t {
    COOLING_SERVICE_ERROR_UNDEFINED = 0x0,
    COOLING_SERVICE_MOTOR_OVERHEAT = 0x1,
    COOLING_SERVICE_CONTROLLER_OVERHEAT = 0x2
};

class PCooling : public IProgram {
    public:
        PCooling(SCar &carService,
                       SSpeed &speedService,
                       IFan* fan, IPump* pump,
                       IMotorController* motorController,
                       IHvEnabled* hvEnabled)
            : _carService(carService), _speedService(speedService) {
            _fan = fan;
            _pump = pump;
            _motorController = motorController;
            _hvEnabled = hvEnabled;

            _pump->setSpeed(0);
            _pump->setEnable(1);
            _fan->setState(FAN_OFF);
        }

        virtual void run() {
            // [QF]
            if (_hvEnabled->read()) {
                speed_value_t currentSpeed = _speedService.getSpeed();

                #ifdef PRINT_SPEED
                    pcSerial.printf("%.3f\n", currentSpeed);
                #endif

                // Activate Fan according to driving speed
                if (currentSpeed <= STD_COOLING_FAN_ON_UNTIL_SPEED) {
                    // Turn fan on if driving too slow
                    _fan->setState(FAN_ON);
                } else if (currentSpeed >= STD_COOLING_FAN_OFF_UNTIL_SPEED) {
                    // Turn fan off if driving too fast
                    _fan->setState(FAN_OFF);
                }

                int16_t motorTemp = 60;//_motorController->getMotorTemp();
                int16_t controllerTemp = 40;//_motorController->getServoTemp();

                // Set Pump speed according to the temperature of the devices
                float pumpMotor = ((float)motorTemp - (float)STD_MOTOR_TEMP_START_PUMP) * 1.0 / ((float)STD_MOTOR_TEMP_FULL_PUMP - (float)STD_MOTOR_TEMP_START_PUMP);
                float pumpController = ((float)controllerTemp - (float)STD_CONTROLLER_TEMP_START_PUMP) * 1.0 / ((float)STD_CONTROLLER_TEMP_FULL_PUMP - (float)STD_CONTROLLER_TEMP_START_PUMP);

                // Use the highest power given from the "hottest" device
                float newPumpPower = 1;
                if (pumpMotor > pumpController) {
                    newPumpPower = pumpMotor;
                } else if (pumpMotor <= pumpController) {
                    newPumpPower = pumpController;
                }

                // Cut power to 0 - 1
                if (newPumpPower > 1.0)
                    newPumpPower = 1.0;
                if (newPumpPower < 0)
                    newPumpPower = 0;

                // Apply pump power
                //_pump->setSpeed(newPumpPower);
                // [QF]
                _pump->setSpeed(1);

                // At least after setting the pump, check if the temperature of any device is too high
                if (motorTemp > STD_MAX_MOTOR_TEMP) {
                    cooling_service_error_type_t motorOverheatError = COOLING_SERVICE_MOTOR_OVERHEAT;
                    _carService.addError(Error(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_COOLING), motorOverheatError, ERROR_CRITICAL));
                }

                if (controllerTemp > STD_MAX_CONTROLLER_TEMP) {
                    cooling_service_error_type_t controllerOverheatError = COOLING_SERVICE_CONTROLLER_OVERHEAT;
                    _carService.addError(Error(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_COOLING), controllerOverheatError, ERROR_CRITICAL));
                }
            } else {
                _pump->setSpeed(0);
                _fan->setState(FAN_OFF);
            }
        }

    protected:
        SCar &_carService;
        SSpeed &_speedService;
        IFan* _fan;
        IPump* _pump;
        IMotorController* _motorController;
        IHvEnabled* _hvEnabled;
};

#endif // PCOOLING_H