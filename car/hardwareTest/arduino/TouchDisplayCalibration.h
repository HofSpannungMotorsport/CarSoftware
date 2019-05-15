#ifndef TOUCH_DISPLAY_CALIBRATION_H
#define TOUCH_DISPLAY_CALIBRATION_H

#include "ILI9341_t3.h"
#include "XPT2046_Touchscreen.h"
#include "../../Bonnie2019/hardware/Pins_Display.h"
#include "tigerLogo.h"

#define BLACK   0x0000
#define WHITE   0xFFFF

ILI9341_t3 display = ILI9341_t3(DISPLAY_TFT_CS, DISPLAY_TFT_DC, DISPLAY_TFT_RST, DISPLAY_TFT_MOSI, DISPLAY_TFT_SCLK, DISPLAY_TFT_MISO);
XPT2046_Touchscreen touch = XPT2046_Touchscreen(DISPLAY_T_CS, DISPLAY_T_IRQ);

static void calibratePoint(uint16_t x, uint16_t y, uint16_t &vi, uint16_t &vj) {
  // Draw cross
  display.drawFastHLine(x - 8, y, 17, WHITE);
  display.drawFastVLine(x, y - 8, 17, WHITE);
  display.drawCircle(x, y, 8, WHITE);

  //Wait until touched
  while (!touch.touched()) {
    delay(10);
  }

  //Get uncallibrated Raw touchpoint
  TS_Point p = touch.getPoint();

  vi = p.x;
  vj = p.y;

  // Erase by overwriting with black
  display.drawFastHLine(x - 8, y, 17, BLACK);
  display.drawFastVLine(x, y - 8, 17, BLACK);
  display.drawCircle(x, y, 8, BLACK);
}

//------------------------------------------------------------------------

void calibrate() {
  uint16_t x1, y1, x2, y2;
  uint16_t vi1, vj1, vi2, vj2;

  //Get calibration-Points
  touch.getCalibrationPoints(x1, y1, x2, y2);
  //Draw and get RAW-Refference
  calibratePoint(x1, y1, vi1, vj1);
  delay(1000);

  //The same again but with new points
  calibratePoint(x2, y2, vi2, vj2);

  TS_Calibration newCalibration = TS_Calibration(vi1, vj1, vi2, vj2);
  touch.setCalibration(newCalibration);

  //Write results into a buffer
  char buf[80];
  snprintf(buf, sizeof(buf), "%d,%d,%d,%d", (int)vi1, (int)vj1, (int)vi2, (int)vj2);

  //Print it to the Display
  display.setCursor(0, 25);
  display.setTextColor(WHITE);
  display.print("setCalibration params:");

  display.setCursor(0, 50);
  display.print(buf);

  display.setCursor(0, 75);
  display.print("Writing to EEPROM...");

  //Save to EEPROM
  //touch.saveCalibrationToEEPROM(newCalibration);

  display.setCursor(0, 100);
  display.print("Finish!");

  display.setCursor(0, 125);
  display.print("Start drawing");

  delay(3000);
}

//------------------------------------------------------------------------

void setupCalibration() {
  delay(1000);

  //Start display and touch
  display.begin();
  touch.begin(display.width(), display.height());

  //Draw background
  display.fillScreen(BLACK);

  //Calibrate
  calibrate();  // No rotation!!

  //Clear Screen
  display.fillScreen(BLACK);
}


void loopCalibration() {
  //If touched than draw
  if (touch.touched()) {
    //Get touched and calibrated position
    TS_Point p = touch.getPixel();

    display.fillCircle(p.x, p.y, 2, WHITE);
  }
}

//------------------------------------------------------------------------

void TouchDisplayCalibration() {
    setupCalibration();

    while(true) {
        loopCalibration();
    }
}

#endif