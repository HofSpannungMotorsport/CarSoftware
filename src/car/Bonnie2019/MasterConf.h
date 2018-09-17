#ifndef BONNIE_2019_MASTER_CONF_H
#define BONNIE_2019_MASTER_CONF_H

#include "mbed.h"
#include "HardwareLed.h"
#include "SoftwareLed.h"
#include "../../can/can_config.h"

CAN can(CAN1_CONF)

SoftwareLed greenLed();
SoftwareLed yellowLed();
SoftwareLed redLed();

#endif