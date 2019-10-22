# Car Software
This is the Software for the Cars at HofSpannung Motorsport e. V.

## Build Status
Master: [![Travis Build Status](https://travis-ci.org/HofSpannungMotorsport/CarSoftware.svg?branch=master)](https://travis-ci.org/HofSpannungMotorsport/CarSoftware)

Development: [![Travis Build Status](https://travis-ci.org/HofSpannungMotorsport/CarSoftware.svg?branch=development)](https://travis-ci.org/HofSpannungMotorsport/CarSoftware)

V0.1 (deprecated): [![Travis Build Status](https://travis-ci.org/HofSpannungMotorsport/CarSoftware.svg?branch=V0.1)](https://travis-ci.org/HofSpannungMotorsport/CarSoftware)

## Functionality
The Carpi is intended to offer a API to communicate with the Car.

### Message Deployment
Throu the Carpi, the message sending and receiving system is built with a complete syncing systeme, to make the message deployment correct and interchangeable between different channels.

### Components
Every Sensor, Input or Output (except for Channels for the Messages) is its own component. Every component gets its own ID and can be synced over the syncing system (not all components have to be syncable).



An up-to-date and detailed README will follow in the next months