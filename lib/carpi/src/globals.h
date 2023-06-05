#pragma once
enum car_state_t : uint8_t
{
    CAR_OFF = 0x0,
    LV_NOT_CALIBRATED = 0x1,
    LV_CALIBRATED = 0x2,
    HV_NOT_CALIBRATED = 0x3,
    HV_CALIBRATED = 0x4,
    LV_CALIBRATING = 0x5,
    HV_CALIBRATING = 0x6,
    DRIVE = 0x7,
    LAUNCH_CONTROL = 0x8,
    CAR_ERROR = 0x9,

};