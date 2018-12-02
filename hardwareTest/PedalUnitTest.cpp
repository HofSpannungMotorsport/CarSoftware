#include "../src/components/hardware/HardwareInterruptButton.h"
#include "../src/components/hardware/HardwarePedal.h"
#include "../src/components/software/SoftwarePedal.h"
#include "../src/can/PedalMessageHandler.h"

#include "../src/can/can_ids.h"
#include "../src/components/interface/IID.h"

#define PEDAL_PIN1 A3
#define PEDAL_PIN2 A4

#define REFRESH_TIME 333 // ms

HardwareInterruptButton calibrationButton(USER_BUTTON);

HardwarePedal hardwarePedal(PEDAL_PIN1, PEDAL_PIN2, PEDAL_GAS);
SoftwarePedal softwarePedal(PEDAL_GAS);

PedalMessageHandler pedalMessageHandler;

#ifndef MESSAGE_REPORT
    Serial pcSerial(USBTX, USBRX); // Connection to PC over Serial
#endif

component_id_t calculateComponentId(void* component) {
    IID *componentId = (IID*)component;
    component_id_t id = ID::getComponentId(componentId->getTelegramTypeId(), componentId->getComponentId());
    return id;
}

void PedalUnitTest() {
    // Pedal Unit Test
    // Calibrate and test a Pedal
    pcSerial.printf("Pedal Unit Test\n");

    while (calibrationButton.getState() != PRESSED);

    pcSerial.printf("Calibration begin");
    softwarePedal.setCalibrationStatus(CURRENTLY_CALIBRATING);
    {
        CANMessage m = CANMessage();
        m.id = calculateComponentId((void*)&softwarePedal);
        pedalMessageHandler.buildMessage((void*)&softwarePedal, m);
        pedalMessageHandler.parseMessage((void*)&hardwarePedal, m);
    }

    while (calibrationButton.getState() != NOT_PRESSED);
    while (calibrationButton.getState() != PRESSED);

    pcSerial.printf("Calibration end");
    softwarePedal.setCalibrationStatus(CURRENTLY_NOT_CALIBRATING);
    {
        CANMessage m = CANMessage();
        m.id = calculateComponentId((void*)&softwarePedal);
        pedalMessageHandler.buildMessage((void*)&softwarePedal, m);
        pedalMessageHandler.parseMessage((void*)&hardwarePedal, m);
    }

    {
        CANMessage m = CANMessage();
        m.id = calculateComponentId((void*)&hardwarePedal);
        pedalMessageHandler.buildMessage((void*)&hardwarePedal, m);
        pedalMessageHandler.parseMessage((void*)&softwarePedal, m);
    }

    if (softwarePedal.getStatus() > 0) {
        pcSerial.printf("Error after Pedal Calibration: %u", softwarePedal.getStatus());
    } else {
        Timer refreshTimer = Timer();
        refreshTimer.start();
        while(softwarePedal.getStatus() == 0) {
            refreshTimer.reset();
            {
                CANMessage m = CANMessage();
                m.id = calculateComponentId((void*)&hardwarePedal);
                pedalMessageHandler.buildMessage((void*)&hardwarePedal, m);
                pedalMessageHandler.parseMessage((void*)&softwarePedal, m);
            }
            pcSerial.printf("Pedal Value: %.3f\n", softwarePedal.getValue());
            while(refreshTimer.read_ms() < REFRESH_TIME);
        }

        pcSerial.printf("Pedal Error: %u", softwarePedal.getStatus());
    }

    pcSerial.printf("End of Program");
    while(1);
}