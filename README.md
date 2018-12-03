# Car Software  [![Travis Build Status](https://travis-ci.org/HofSpannungMotorsport/CarSoftware.svg?branch=master)](https://travis-ci.org/HofSpannungMotorsport/CarSoftware)
This is the Software for our Cars at HofSpannung Motorsport e. V.

## Development
For now, we build the core of our Software to provide basic functionality. After finishing this, the goal is the new Software for our next car, Bonnie

## Functionality
### Bonnie
The Software controls the whole Car. The Buttons and LEDs in the Dashpanel, the Brake and Gas Pedal, the Spring-Travel-Sensors and the Brake-Light will be collected and controlled from this Software, including the control of the Motor.

The Software will run on the following Devices, including the Structure of the wireing (Communication over CAN):

Master Controller
- Brake-Light
- Buzzer
- Fan
- Pump
- Spring-Travel-Sensor Rear left and right
- RPM-Sensor Rear left and right

Dash-Panel-Slave
- LED 1 to 3
- Button 1 and 2

Pedal-Slave
- Gas Pedal
- Brake Pedal
- Spring-Travel-Sensor Front left and right
- Steering-Angle-Sensor
- RPM-Sensor Front left and right

## Current Schedule
### November 2018
Software finish for Bonnie

### December 2018
Software Testing
