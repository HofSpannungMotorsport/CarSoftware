#pragma once
enum car_state_t : uint8_t
{
    CAR_OFF = 0x0,
    LV_NOT_CALIBRATED = 0x1,
    LV_CALIBRATED = 0x2,
    HV_NOT_CALIBRATED = 0x3,
    HV_CALIBRATED = 0x4,
    CALIBRATING = 0x5,
    DRIVE = 0x6,
    LAUNCH_CONTROL = 0x7,
    CAR_ERROR = 0x8

};