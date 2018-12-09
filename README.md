# Car Software  [![Travis Build Status](https://travis-ci.org/HofSpannungMotorsport/CarSoftware.svg?branch=master)](https://travis-ci.org/HofSpannungMotorsport/CarSoftware)
This is the Software for the Cars at HofSpannung Motorsport e. V.

## Development
For now, we build the core and framework of our Software to provide basic functionality. After finishing this, the goal is the new Software for our next car, Bonnie.

## Functionality
### Bonnie
The Software controls the whole Car. The Buttons and LEDs in the Dashpanel, the Brake and Gas Pedal, the Spring-Travel-Sensors and the Brake-Light will be collected and controlled from this Software, including the control of the Motor.

The Software will run on the following Devices (Car-intern communication over CAN):

Master Controller
- Dout Brake-Light
- Dout Buzzer
- Dout Cooling Fan
- PWMoutCooling Pump
- AnalogIn Spring-Travel-Sensor Rear left and right
- Din RPM-Sensor Rear left and right
- Motor Controller (CAN, Dout RFE enable, Dout RUN enable)
- Din HV-Enable input

Dash-Panel-Slave
- PWMout LED 1 to 3
- Din Button 1 and 2

Pedal-Slave
- AnalogIn Gas Pedal (2 Sensors)
- AnalogIn Brake Pedal
- AnalogIn Spring-Travel-Sensor Front left and right
- AnalogIn Steering-Angle-Sensor
- Din RPM-Sensor Front left and right

## Current Schedule
### November 2018
Software finish for Bonnie ✓

### December 2018
Software Testing
