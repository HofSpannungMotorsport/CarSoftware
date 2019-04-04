//#define PEDAL_MESSAGE_HANDLER_DEBUG
#include "carpi.h"

#define PEDAL_PIN1 PC_0
#define PEDAL_PIN2 PC_1

#define REFRESH_TIME 20 // ms

HardwareInterruptButton calibrationButton(USER_BUTTON);

HardwarePedal hardwarePedal(PEDAL_PIN1, /*PEDAL_PIN2,*/ PEDAL_BRAKE);
SoftwarePedal softwarePedal(PEDAL_GAS);

PedalMessageHandler pedalMessageHandler;

message_id_t calculateComponentId(void* component) {
    IComponent *componentId = (IComponent*)component;
    component_id_t id = ID::getMessageId(NORMAL, ID::getComponentId(componentId->getTelegramTypeId(), componentId->getComponentId()), componentId->getObjectType());
    return id;
}

void PedalUnitTest() {
    // Pedal Unit Test
    // Calibrate and test a Pedal
    pcSerial.printf("Pedal Unit Test\n");

    while (calibrationButton.getState() != PRESSED);

    pcSerial.printf("Calibration begin\n");
    softwarePedal.setCalibrationStatus(CURRENTLY_CALIBRATING);
    {
        CANMessage m = CANMessage();
        m.id = calculateComponentId((void*)&softwarePedal);
        pedalMessageHandler.buildMessage((void*)&softwarePedal, m);
        pedalMessageHandler.parseMessage((void*)&hardwarePedal, m);
    }

    while (calibrationButton.getState() != NOT_PRESSED);
    while (calibrationButton.getState() != PRESSED);

    pcSerial.printf("Calibration end\n");
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
        pcSerial.printf("Error after Pedal Calibration: 0x%x", softwarePedal.getStatus());
    } else {
        Timer refreshTimer;
        refreshTimer.start();
        while(softwarePedal.getStatus() == 0) {
            refreshTimer.reset();
            float currentHardwarePedalValue = hardwarePedal.getValue();
            {
                CANMessage m = CANMessage();
                m.id = calculateComponentId((void*)&hardwarePedal);
                pedalMessageHandler.buildMessage((void*)&hardwarePedal, m);
                pedalMessageHandler.parseMessage((void*)&softwarePedal, m);
            }
            pcSerial.printf("%.4f\t%.4f\n", softwarePedal.getValue(), currentHardwarePedalValue);
            while(refreshTimer.read_ms() < REFRESH_TIME);
        }

        pcSerial.printf("Pedal Error: 0x%x\n", softwarePedal.getStatus());
    }

    pcSerial.printf("\n\n-----\nEnd of Program");
    while(1);
}