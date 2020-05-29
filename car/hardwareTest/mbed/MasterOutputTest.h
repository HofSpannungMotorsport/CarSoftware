#include "carpi.h"
#include "../../Bonnie2019/hardware/Pins_Master_PCB.h"
#include <string>


HardwareInterruptButton buttonNext(MASTER_PIN_USER_BUTTON, COMPONENT_BUTTON_START);

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
DigitalIn enabledHV(MASTER_PIN_HV_ENABLED_TSMS);

// Din RPM Sensors
//DigitalIn rpmHL(MASTER_PIN_RPM_SENSOR_HL);
//DigitalIn rpmHR(MASTER_PIN_RPM_SENSOR_HR);

// Dout LED HV Enabled
DigitalOut ledHVEnabled(MASTER_PIN_LED_HV_ENABLED);

// Dout LED Error
DigitalOut ledError(MASTER_PIN_LED_ERROR);

// DOUT LED Extra
DigitalOut ledExtra(MASTER_PIN_LED_ALIVE);

void waitForClick() {
    while(buttonNext.getState() != NOT_PRESSED);
    while(buttonNext.getState() != PRESSED);
}

void testPin(string deviceName, DigitalOut &device) {
    string message = "Click button to turn On:  " + deviceName + "\n";
    printf("%s", message.c_str());
    waitForClick();
    device = 1;

    message = "Click button to turn Off: " + deviceName + "\n";
    printf("%s", message.c_str());
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

void MasterOutputTest() {
    /*
       Hi :)
       Today we want to test the outputs of the master
       Will the relais work?
       Has Anton made the right desicions by soldering while drinking?
       Will we see great sparks?
      
       Let's Start our Test!!! :DDDDD
    */

    // At first, release all
    releaseAll();

    printf("\nHi! I'm Bert. I am your pin test bot. Let's Start! Just press the Blue Button on the Master Board\n\n");
    waitForClick();

    printf("Current input Values:\n\tSpringTravelSensor HL: %i\n\tSpringTravelSensor HR: %i\n\tHV Enable: %i\n\n",
        springTravelSensorHL.read_u16(), springTravelSensorHR.read_u16(), enabledHV.read()/*, rpmHL.read(), rpmHR.read()*/);


    testPin("Buzzer", buzzer);

    string message = "Click button to turn On:  RFE and than RUN\n";
    printf("%s", message.c_str());
    waitForClick();
    enableRFE = 1;
    wait(0.5);
    enableRUN = 1;

    message = "Click button to turn Off: RFE and RUN\n";
    printf("%s", message.c_str());
    waitForClick();
    enableRFE = 0;
    enableRUN = 0;


    testPin("Brake Light", brakeLight);
    testPin("Pump Enable", pumpEnable);

    printf("Click button to turn On:  Pump PWM\n");
    waitForClick();
    pumpPWM = 1;

    printf("Click button to turn Off: Pump PWM\n");
    waitForClick();
    pumpPWM = 0;

    testPin("Cooling Fans", coolingFans);
    testPin("LED HV Enabled", ledHVEnabled);
    testPin("LED Error", ledError);
    testPin("LED Extra", ledExtra);

    printf("Finish! Hope everything was right. If not, look in the Pins_Master.h\n\nStarting all over again :D\n\n");
}