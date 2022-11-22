#ifndef DISPLAYSLAVECONF_H
#define DISPLAYSLAVECONF_H

//#define CAN_DEBUG
#include "carpi.h"

#include "hardware/Pins_Display.h"

#define DISPLAY_SEND_RATE 3 // Hz

CANService canService(CAN_RX,CAN_TX);

HardwareDisplay display(COMPONENT_DISPLAY_MAIN);



class Display : public Carpi {
    public:
        // Called once at bootup
        void setup() {
            
            canService.setSenderId(DEVICE_DISPLAY);

            canService.addComponent((ICommunication*)&display);

            wait(0.1);
        }
    
        // Called repeately after bootup
        void loop() {
            canService.run();

            wait(1.0/(float)DISPLAY_SEND_RATE);
        }
};

Display runtime;


#endif
