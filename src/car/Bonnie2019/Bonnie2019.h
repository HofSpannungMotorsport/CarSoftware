#ifndef BONNIE_2019_H
#define BONNIE_2019_H

#include "mbed.h"

#ifdef BOARD_DASHBOARD
#include "DashboardSlaveConf.h"
#endif

#ifdef BOARD_PEDAL
#include "PedalSlaveConf.h"
#endif

#ifdef BOARD_MASTER
#include "MasterConf.h"
#endif

void initCar()
{
    // assign the components to CANService here
}

#endif.