#include "SPI.h"
#include "ILI9341_t3.h"
#include "../../Bonnie2019/hardware/Pins_Display.h"
#include "tigerLogo.h"

#define LIGHTUP_TIME 1 // s
#define TRANSITION_ITERATIONS 30
#define TRANSITION_TIME 1.5 // s
#define ROTATION 0

ILI9341_t3 tft = ILI9341_t3(DISPLAY_TFT_CS, DISPLAY_TFT_DC, DISPLAY_TFT_RST, DISPLAY_TFT_MOSI, DISPLAY_TFT_SCLK, DISPLAY_TFT_MISO);


void DisplayLogoTest() {
    tft.begin();
    tft.setRotation(ROTATION);

    pinMode(DISPLAY_TFT_BACKLIGHT, OUTPUT);
    analogWrite(DISPLAY_TFT_BACKLIGHT, 0);

    tft.fillScreen(ILI9341_BLACK);

    delay(1);

    tft.drawBitmap(120 - (tigerLogoMeta.width/2), 160 - (tigerLogoMeta.hight/2), tigerLogo, tigerLogoMeta.width, tigerLogoMeta.hight, ILI9341_WHITE);

    unsigned long startTime = millis();
    while (millis() - startTime < (float)LIGHTUP_TIME*1000.0);

    for (uint8_t i = 0; i < TRANSITION_ITERATIONS; i++) {
        startTime = millis();

        float brightness = ((float)i / (float)TRANSITION_ITERATIONS)*255;

        /*
        uint16_t colorTiger = ((uint16_t)((1-brightness) * 0x1F) & 0x1F) << 11;
        colorTiger |= ((uint16_t)((1-brightness) * 0x2F) & 0x2F) << 5;
        colorTiger |= ((uint16_t)((1-brightness) * 0x1F) & 0x1F);
        */

        uint16_t colorTiger = tft.color565(255-brightness, 255-brightness, 255-brightness);

        /*
        uint16_t colorBackground = ((uint16_t)(brightness * 0x1F) & 0x1F) << 11;
        colorBackground |= ((uint16_t)(brightness * 0x2F) & 0x2F) << 5;
        colorBackground |= ((uint16_t)(brightness * 0x1F) & 0x1F);
        */

       uint16_t colorBackground = tft.color565(brightness, brightness, brightness);

        tft.setRotation(1);
        tft.fillScreen(colorBackground);
        tft.setRotation(0);
        tft.drawBitmap(120 - (tigerLogoMeta.width/2), 160 - (tigerLogoMeta.hight/2), tigerLogo, tigerLogoMeta.width, tigerLogoMeta.hight, colorTiger);

        while (millis() - startTime < (((float)TRANSITION_TIME/(float)TRANSITION_ITERATIONS)*1000.0));
    }

    tft.fillScreen(ILI9341_WHITE);
    tft.drawBitmap(120 - (tigerLogoMeta.width/2), 160 - (tigerLogoMeta.hight/2), tigerLogo, tigerLogoMeta.width, tigerLogoMeta.hight, ILI9341_BLACK);



    while(true);
}