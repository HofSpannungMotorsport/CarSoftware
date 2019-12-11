#ifndef PCOOLING_H
#define PCOOLING_H

#include "IProgram.h"
#include "communication/componentIds.h"
#include "services/SCar.h"
#include "services/SSpeed.h"
#include "components/interface/IFan.h"
#include "components/interface/IPump.h"
#include "components/interface/IMotorController.h"

enum cooling_service_error_type_t {
    COOLING_SERVICE_OK = 0x0,
    COOLING_SERVICE_MOTOR_OVERHEAT = 0x1,
    COOLING_SERVICE_CONTROLLER_OVERHEAT = 0x2
};

class PCooling : public IProgram {
    public:
        PCooling(SCar &carService,
                 SSpeed &speedService,
                 IFan &fan, IPump &pump,
                 IMotorController &motorController,
                 IHvEnabled &hvEnabled,
                 IRegistry &registry)
            : _registry(registry),
              _carService(carService), _speedService(speedService),
              _fan(fan), _pump(pump), _motorController(motorController), _hvEnabled(hvEnabled) {

            _pump.setSpeed(0);
            _pump.setEnable(1);
            _fan.setState(FAN_OFF);
        }

        virtual void run() {
            // [QF]
            if (_hvEnabled.read()) {
                speed_value_t currentSpeed = _speedService.getSpeed();

                #ifdef PRINT_SPEED
                    printf("%.3f\n", currentSpeed);
                #endif

                // Activate Fan according to driving speed
                if (currentSpeed <= _registry.getFloat(COOLING_FAN_ON_UNITL_SPEED)) {
                    // Turn fan on if driving too slow
                    _fan.setState(FAN_ON);
                } else if (currentSpeed >= _registry.getFloat(COOLING_FAN_OFF_UNTIL_SPEED)) {
                    // Turn fan off if driving too fast
                    _fan.setState(FAN_OFF);
                }

                float motorTemp = _motorController.getMotorTemp();
                float controllerTemp = _motorController.getServoTemp();

                // Set Pump speed according to the temperature of the devices
                float motorTempStartPump = _registry.getFloat(COOLING_MOTOR_TEMP_START_PUMP);
                float motorTempFullPump = _registry.getFloat(COOLING_MOTOR_TEMP_FULL_PUMP);

                float controllerTempStartPump = _registry.getFloat(COOLING_CONTROLLER_TEMP_START_PUMP);
                float controllerTempFullPump = _registry.getFloat(COOLING_CONTROLLER_TEMP_FULL_PUMP);

                float pumpMotor = (motorTemp - motorTempStartPump) / (motorTempFullPump - motorTempStartPump);
                float pumpController = (controllerTemp - controllerTempStartPump) / (controllerTempFullPump - controllerTempStartPump);

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
                _pump.setSpeed(newPumpPower);

                // At least after setting the pump, check if the temperature of any device is too high
                if (motorTemp > _registry.getFloat(COOLING_MAX_MOTOR_TEMP)) {
                    cooling_service_error_type_t motorOverheatError = COOLING_SERVICE_MOTOR_OVERHEAT;
                    _carService.addError(Error(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_COOLING), motorOverheatError, ERROR_CRITICAL));
                }

                if (controllerTemp > _registry.getFloat(COOLING_MAX_CONTROLLER_TEMP)) {
                    cooling_service_error_type_t controllerOverheatError = COOLING_SERVICE_CONTROLLER_OVERHEAT;
                    _carService.addError(Error(componentId::getComponentId(COMPONENT_SYSTEM, COMPONENT_SYSTEM_COOLING), controllerOverheatError, ERROR_CRITICAL));
                }
            } else {
                _pump.setSpeed(0);
                _fan.setState(FAN_OFF);
            }
        }

    protected:
        IRegistry &_registry;
    
        SCar &_carService;
        SSpeed &_speedService;
        IFan &_fan;
        IPump &_pump;
        IMotorController &_motorController;
        IHvEnabled &_hvEnabled;
};

#endif // PCOOLING_H