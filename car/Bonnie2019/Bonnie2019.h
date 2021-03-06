#ifndef BONNIE_2019_H
#define BONNIE_2019_H

#include "SanityCheck.h"

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

#ifdef BOARD_DASHBOARD_DEMO
#include "DashboardDemoConf.h"
#endif

#endif