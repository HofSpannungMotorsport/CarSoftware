#ifndef BONNIE_2019_H
#define BONNIE_2019_H

#include "SanityCheck.h"

#define LOOP_WAIT_TIME 0.0000001
#define STARTUP_WAIT_TIME_MASTER 0.8
#define STARTUP_WAIT_TIME_SLAVE 0.3

#ifdef BOARD_DASHBOARD
#include "DashboardSlaveConf.h"
#endif

#ifdef BOARD_PEDAL
#include "PedalSlaveConf.h"
#endif

#ifdef BOARD_MASTER
#include "MasterConf.h"
#endif

#ifdef BOARD_DISPLAY
#include "DisplaySlaveConf.h"
#endif

#ifdef BOARD_LORA
#include "LoRaConf.h"
#endif

#endif