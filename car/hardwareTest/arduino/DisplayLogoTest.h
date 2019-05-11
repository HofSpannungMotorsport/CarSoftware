#include "SPI.h"
#include "ILI9341_t3.h"
#include "XPT2046_Touchscreen.h"
#include "../../Bonnie2019/hardware/Pins_Display.h"
#include "tigerLogo.h"

#define LIGHTUP_TIME 2 // s
#define TRANSITION_TIME 1 // s
#define ROTATION 0

ILI9341_t3 tft = ILI9341_t3(DISPLAY_TFT_CS, DISPLAY_TFT_DC, DISPLAY_TFT_RST, DISPLAY_TFT_MOSI, DISPLAY_TFT_SCLK, DISPLAY_TFT_MISO);

void waitSeconds(float s) {
    unsigned long startTime = millis();
    while (startTime - millis() < (s*1000.0));
}

void DisplayLogoTest() {
    tft.begin();
    tft.setRotation(ROTATION);

    pinMode(DISPLAY_TFT_BACKLIGHT, OUTPUT);
    analogWrite(DISPLAY_TFT_BACKLIGHT, 0);

    tft.fillScreen(ILI9341_BLACK);

    delay(1000);

    tft.drawBitmap(120 - (tigerLogoMeta.width/2), 160 - (tigerLogoMeta.hight/2), tigerLogo, tigerLogoMeta.width, tigerLogoMeta.hight, ILI9341_WHITE);

    for (uint8_t i = 0; i <= 0xFF; i++) {
        unsigned long startTime = millis();

        analogWrite(DISPLAY_TFT_BACKLIGHT, i);
        waitSeconds((float)LIGHTUP_TIME/255.0);


        while (startTime - millis() < (((float)LIGHTUP_TIME/255.0)*1000.0));
    }

    analogWrite(DISPLAY_TFT_BACKLIGHT, 255);

    for (uint8_t i = 0; i <= 0xFF; i++) {
        unsigned long startTime = millis();

        float brightness = (float)i / 255.0;

        uint16_t colorTiger = ((uint16_t)((1-brightness) * 0x1F) & 0x1F) << 11;
        colorTiger |= ((uint16_t)((1-brightness) * 0x2F) & 0x2F) << 5;
        colorTiger |= ((uint16_t)((1-brightness) * 0x1F) & 0x1F);

        uint16_t colorBackground = ((uint16_t)(brightness * 0x1F) & 0x1F) << 11;
        colorBackground |= ((uint16_t)(brightness * 0x2F) & 0x2F) << 5;
        colorBackground |= ((uint16_t)(brightness * 0x1F) & 0x1F);

        tft.fillScreen(colorBackground);
        tft.drawBitmap(120 - (tigerLogoMeta.width/2), 160 - (tigerLogoMeta.hight/2), tigerLogo, tigerLogoMeta.width, tigerLogoMeta.hight, colorTiger);

        while (startTime - millis() < (((float)TRANSITION_TIME/255.0)*1000.0));
    }

    while(true);
}