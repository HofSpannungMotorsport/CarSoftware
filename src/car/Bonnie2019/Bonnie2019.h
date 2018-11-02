#ifndef BONNIE_2019_H
#define BONNIE_2019_H

#include "SanityCheck.h"

#include "../../can/can_config.h"
#include "../../can/CANService.h"
CAN can(CAN1_CONF);
CANService canService;

#ifdef BOARD_DASHBOARD
#include "DashboardSlaveConf.h"
#endif

#ifdef BOARD_PEDAL
#include "PedalSlaveConf.h"
#endif

#ifdef BOARD_MASTER
#include "MasterConf.h"
#endif

#endif