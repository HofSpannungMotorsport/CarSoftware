#include "../src/can/can_ids.h"
#include "../src/car/Bonnie2019/hardware/Pins_Master.h"
#include "../src/components/hardware/HardwareInterruptButton.h"
#include "mbed.h"
#include <string>


HardwareInterruptButton buttonNext(MASTER_PIN_USER_BUTTON, BUTTON_START);

// AnalogIn Spring Travel Sensor
AnalogIn springTravelSensorHL(MASTER_PIN_SPRING_TRAVEL_SENSOR_HL);
AnalogIn springTravelSensorHR(MASTER_PIN_SPRING_TRAVEL_SENSOR_HR);

// Dout Buzzer
DigitalOut buzzer(MASTER_PIN_BUZZER);

// Dout Enable Pins for Motor Controller
DigitalOut enableRFE(MASTER_PIN_RFE_ENABLE);
DigitalOut enableRUN(MASTER_PIN_RUN_ENABLE);

// Dout Brake Light
DigitalOut brakeLight(MASTER_PIN_BRAKE_LIGHT);

// Dout Pump enable
DigitalOut pumpEnable(MASTER_PIN_PUMP_ENABLE);

// PWMOut Pump
PwmOut pumpPWM(MASTER_PIN_PUMP_PWM);

// Cooling Fans
DigitalOut coolingFans(MASTER_PIN_FAN);

// HV Enable
DigitalIn enabledHV(MASTER_PIN_HV_ENABLED);

// Din RPM Sensors
DigitalIn rpmHL(MASTER_PIN_RPM_SENSOR_HL);
DigitalIn rpmHR(MASTER_PIN_RPM_SENSOR_HR);

// Dout LED HV Enabled
DigitalOut ledHVEnabled(MASTER_PIN_LED_HV_ENABLED);

// Dout LED Error
DigitalOut ledError(MASTER_PIN_LED_ERROR);

// DOUT LED Extra
DigitalOut ledExtra(MASTER_PIN_LED_EXTRA);

void waitForClick() {
    while(buttonNext.getState() != NOT_PRESSED);
    while(buttonNext.getState() != PRESSED);
}

void testPin(string deviceName, DigitalOut &device) {
    string message = "Click button to turn On:  " + deviceName + "\n";
    pcSerial.printf("%s", message.c_str());
    waitForClick();
    device = 1;

    message = "Click button to turn Off: " + deviceName + "\n";
    pcSerial.printf("%s", message.c_str());
    waitForClick();
    device = 0;
}

void releaseAll() {
    buzzer = 0;
    enableRFE = 0;
    enableRUN = 0;
    brakeLight = 0;
    pumpEnable = 0;
    pumpPWM = 0;
    coolingFans = 0;
    ledHVEnabled = 0;
    ledError = 0;
    ledExtra = 0;
}

void RFEandRUNtest() {
    // At first, release all
    releaseAll();

    pcSerial.printf("Click button to turn On:  RFE\n");
    waitForClick();
    enableRFE = 1;

    pcSerial.printf("Click button to turn On:  RUN\n");
    waitForClick();
    enableRUN = 1;

    pcSerial.printf("Click button to turn Off: RFE and RUN\n\n");
    waitForClick();
    enableRFE = 0;
    enableRUN = 0;
}