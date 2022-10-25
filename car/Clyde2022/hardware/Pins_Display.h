#ifndef PINS_DISPLAY_H
#define PINS_DISPLAY_H

// Same as old one, only new pins
//TOUCH
#define DISPLAY_TOUCH_IRQ D2
#define DISPLAY_TOUCH_CS D8

//DISPLAY
#define DISPLAY_TFT_RST D6
#define DISPLAY_TFT_DC D9
#define DISPLAY_TFT_CS D10
#define DISPLAY_TFT_MOSI D11
#define DISPLAY_TFT_MISO D12
#define DISPLAY_TFT_CLK D13

//LED1
#define DISPLAY_LED1_R D5
#define DISPLAY_LED1_B D7
#define DISPLAY_LED1_G D14

//LED2
#define DISPLAY_LED2_B D15
#define DISPLAY_LED2_R D16
#define DISPLAY_LED2_G D17

//LED3
#define DISPLAY_LED3_B D18
#define DISPLAY_LED3_R D19
#define DISPLAY_LED3_G D20

//LED4
#define DISPLAY_LED4_B D21
#define DISPLAY_LED4_R D22
#define DISPLAY_LED4_G D23

#define DISPLAY_CAN D3,D4
//#define CAN_TX 3
//#define CAN_RX 4

#endif // PINS_DISPLAY_H