#ifndef DISPLAYSLAVECONF_H
#define DISPLAYSLAVECONF_H

// #include "carpi.h"

#include "hardware/Pins_Display.h"
#include <FlexCAN.h>
#include <kinetis_flexcan.h>

// create CAN object
FlexCAN CANReceiver(100000);
class Display
{

public:
    CAN_message_t msg;

    void setup()
    {
        // init CAN bus
        CANReceiver.begin();
        delay(1000);
        Serial.println("CAN Receiver Initialized");
    }

    void loop()
    {
        while (CANReceiver.read(msg))
        {
            // Serial.println("ID: " + msg.buf[0]);

            // toggle LEDs
            if (msg.buf[0] == 16)
            {
                if (msg.buf[1] == 0)
                {
                    Serial.print("Status ");
                    Serial.println(msg.buf[2]);

                    Serial.print("State ");
                    Serial.println(msg.buf[3]);

                    uint16_t speed16 = ((uint16_t)msg.buf[4] << 8) | (uint16_t)msg.buf[5];
                    float speed = (float)speed16 / 100.0;
                    // this->setSpeed(speed);
                    Serial.print("Speed ");
                    Serial.print(speed);
                    Serial.println("km/h");

                    uint16_t current16 = ((uint16_t)msg.buf[6] << 8) | (uint16_t)msg.buf[7];
                    float current = (float)current16 / 100.0;
                    // this->setCurrent(current);
                    Serial.print("Current ");
                    Serial.print(current);
                    Serial.println("A");
                }
                else if (msg.buf[1] == 1)
                {
                    Serial.print("MotorTemp ");
                    Serial.println(msg.buf[2]);

                    Serial.print("AirTemp ");
                    Serial.println(msg.buf[3]);

                    uint16_t dcVoltage16 = ((uint16_t)msg.buf[4] << 8) | (uint16_t)msg.buf[5];
                    float dcVoltage = (float)dcVoltage16 / 100.0;
                    // this->setVoltage(dcVoltage);
                    Serial.print("DC-Voltage ");
                    Serial.print(dcVoltage);
                    Serial.print("V\n\n\n");
                }
            }
        }
    }
};

Display runtime;

#endif