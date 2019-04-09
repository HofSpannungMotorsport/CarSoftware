# Car Software  [![Travis Build Status](https://travis-ci.org/HofSpannungMotorsport/CarSoftware.svg?branch=master)](https://travis-ci.org/HofSpannungMotorsport/CarSoftware)
This is the Software for the Cars at HofSpannung Motorsport e. V.

## Development
For now, we build the core and API of our Software to provide basic functionality. After finishing this, the goal is the new Software for our next cars, Bonnie and Clyde.

## Software Structure
The Software consists of two Parts: The API and the actual Software for the individual Cars.

The API provides all components, like the pedals or buttons, and also provides basic services and programs, like a speed Service or syncing system for CAN or Serial (RS232).

The Software for the individual Cars then provides components and programs, which will be just appliable to the dedicated Car, like the Motor Controller Program.

## Functionality
### Bonnie
The Software controls the whole Car. The Buttons and LEDs in the Dashpanel, the Brake and Gas Pedal, the Spring-Travel-Sensors and the Brake-Light will be collected and controlled from this Software, including the control of the Motor. The Cooling System is also controlled according to the speed and temperature of the car and individual components.

The Software will run on the following Devices (Car-intern communication over CAN):

Master Controller
- Dout Brake-Light
- Dout Buzzer
- Dout Cooling Fan
- PWMout Cooling Pump
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

For the Dashpanel and Pedal Slave there are custom made PCBs to provide voltage divider and CAN Transreceiver on one Shield. The Port/Pin definitions can be found in car/Bonnie2019/hardware/.

## Current Schedule
### November 2018
Software finish for Bonnie ✓

### December 2018 - March 2019
Software Testing ✓

### April 2019 - June 2019
Car Testing on Track including all safety mechanisms of the Software ~
