#ifndef HARDWAREPEDAL_H
#define HARDWAREPEDAL_H

#include "IPedal.h"
#include "mbed.h"

class HardwarePedal : public IPedal
{

    public:

        HardwarePedal()
        {

        }

        virtual ~HardwarePedal()
        {
            
        }

        virtual pedal_status_t getStatus()
        {

        }

        virtual pedal_value_t getValue()
        {

        }
        
        virtual void setStatus(pedal_status_t status)
        {
            // no implementation needed
        }
        
        virtual void setValue(pedal_value_t value)
        {
            // no implementation needed
        }
        
};

#endif