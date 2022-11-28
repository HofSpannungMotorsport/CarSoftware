#ifndef HARDWAREDISPLAY_H
#define HARDWAREDISPLAY_H

#include "../interface/IDisplay.h"

class HardwareDisplay : public IDisplay {
    public:
        HardwareDisplay(){
            setComponentType(COMPONENT_DISPLAY);
            setObjectType(OBJECT_HARDWARE);
        }

        HardwareDisplay(id_sub_component_t componentSubId)
            : HardwareDisplay(){
                setComponentSubId(componentSubId);
        }
        
        uint16_t setMinCellVoltage(uint16_t value){
            _minCellVoltage = value;
        }

        uint16_t setMaxCellVoltage(uint16_t value){
             _maxCellVoltage = value;
        }

        uint16_t setMinCellTemperature(uint16_t value){
             _minCellTemperature = value;
        }

        uint16_t setMaxCellTemperature(uint16_t value){
            _maxCellTemperature = value;
        }
        
        float setSpeed(float value){
            _speed = value;
        }

        float setCurrent(float value){
             _current = value;
        }

        uint16_t setMotorTemp(uint16_t value){
             _motorTemp = value;
        }

        uint16_t setAirTemp(uint16_t value){
            _airTemp = value;
        }

        float setDcVoltage(float value){
             _dcVoltage = value;
        }
        
        display_status_t getStatus(){
            return _status;
        }

        uint16_t getMinCellVoltage(){
            return _minCellVoltage;
        }

        uint16_t getMaxCellVoltage(){
            return _maxCellVoltage;
        }

        uint16_t getMinCellTemperature(){
            return _minCellTemperature;
        }

        uint16_t getMaxCellTemperature(){
            return _maxCellTemperature;
        }

        virtual message_build_result_t buildMessage(CarMessage &carMessage) {
            car_sub_message_t subMessage;


            return MESSAGE_BUILD_OK;
        }

        virtual message_parse_result_t parseMessage(CarMessage &carMessage) {
            message_parse_result_t result = MESSAGE_PARSE_OK;
            for (car_sub_message_t &subMessage : carMessage.subMessages) {
                if(subMessage.length != 5) // not a valid message
                    result = MESSAGE_PARSE_ERROR;

                uint16_t cmdId = subMessage.data[0];
                switch (cmdId)
                {
                case 0:
                    {
                    this->setMinCellVoltage(subMessage.data[1]);
                    this->setMaxCellVoltage(subMessage.data[2]);
                    this->setMinCellTemperature(subMessage.data[3]);
                    this->setMaxCellTemperature(subMessage.data[4]);

                    pcSerial.printf("[Display]@parseMessage: CMD: %x\tMinV:%x\tMaxV:%x\tMinT%x\tMaxT:%x", subMessage.data[0],subMessage.data[1],subMessage.data[2], subMessage.data[3], subMessage.data[4]);

                    }
                    break;
                case 1:
                    {
                    
                    uint16_t speed16 = subMessage.data[1] | (subMessage.data[2] << 8);
                    float speed = (float)speed16 / 65535.0;
                    this->setSpeed(speed);

                    uint16_t current16 = subMessage.data[3] | (subMessage.data[4] << 8);
                    float current = (float)current16 / 65535.0;
                    this->setCurrent(current);
                    }
                    break;
                case 2:
                    {

                        this->setMotorTemp(subMessage.data[1]);
                        this->setAirTemp(subMessage.data[2]);

                        uint16_t dcVoltage16 = subMessage.data[3] | (subMessage.data[4] << 8);
                        float dcVoltage = (float)dcVoltage16 / 65535.0;
                        this->setDcVoltage(dcVoltage);
                    }
                    
                    break;
                
                default:
                    result = MESSAGE_PARSE_ERROR;
                    break;
                }

            }
            
            return result;
        }

    protected:
        display_status_t _status = 0;
        bool _ready = false;
        bool _calibrationSet = false;

        //ACCUMULATOR
        uint16_t _minCellVoltage = 0;
        uint16_t _maxCellVoltage = 0;
        uint16_t _minCellTemperature = 0;
        uint16_t _maxCellTemperature = 0;


        //MOTORCONTROLLER
        float _speed = 0.0;
        float _current = 0.0;
        uint16_t _motorTemp = 0;
        uint16_t _airTemp = 0;
        float _dcVoltage = 0.0;

        /*
        HARDWAREMOTORCONTROLLER
        -rfe
        -run        
        */
};

#endif