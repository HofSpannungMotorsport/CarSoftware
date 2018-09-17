#ifndef BONNIE_2019_DASHBOARD_CONF_H
#define BONNIE_2019_DASHBOARD_CONF_H

#include "HardwareLed.h"
#include "mbed.h"
#include "../../can/can_config.h"
#include "../../can/LEDMessaeHandler.h"

static LEDMessaeHandler ledBridge;
//static ButtonMessaeHandler ledBridge;

CAN can(CAN1_CONF);
//CANService canService...

HardwareLed ledGreen(D6);
HardwareLed ledYellow(D5);
HardwareLed ledRed(D3);


// Buttons
//BTN-Oins: D9, D7

void initBoardHardware()
{
    // assign the components to CANService here
    
}

#endif
