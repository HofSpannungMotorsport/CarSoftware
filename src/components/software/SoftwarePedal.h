#ifndef SOFTWAREPEDAL_H
#define SOFTWAREPEDAL_H

#include "../interface/IPedal.h"

class SoftwarePedal : public IPedal
{
    private:
        pedal_status_t _status;
        pedal_value_t _value;

    public:
        virtual pedal_status_t getStatus()
        {
            return _status;
        }

        virtual pedal_value_t getValue()
        {
            return _value;
        }
        
        virtual void setStatus(pedal_status_t status)
        {
            _status = status;
        }
        
        virtual void setValue(pedal_value_t value)
        {
            _value = value;
        }

        
        
};

#endif