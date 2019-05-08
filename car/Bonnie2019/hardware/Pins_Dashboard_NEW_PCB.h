#ifndef PINS_DASHBOARD_H
#define PINS_DASHBOARD_H

// Same as old one, only new pins

#define DASHBOARD_PIN_BUTTON_RESET D12
#define DASHBOARD_PIN_BUTTON_START D11
#define DASHBOARD_PIN_LED_RED      D3 // PWM -> Marked as LED Green on PCB!
#define DASHBOARD_PIN_LED_YELLOW   D6 // PWM -> Marked as LED Yellow on PCB!
#define DASHBOARD_PIN_LED_GREEN    D9 // PWM -> Marked as LED Red on PCB!

#define DASHBOARD_CAN D10,D2

// New components since this PCB

#define DASHBOARD_PIN_SERIAL_TX D1
#define DASHBOARD_PIN_SERIAL_RX D0
#define DASHBOARD_PIN_LED_EXTRA A6
#define DASHBOARD_PIN_BUTTON_RESET_LED A1
#define DASHBOARD_PIN_BUTTON_START_LED A2

#endif // PINS_DASHBOARD_H