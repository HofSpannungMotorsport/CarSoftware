#ifndef SOFTWAREDISPLAY_H
#define SOFTWAREDISPLAY_H

#include "../interface/IDisplay.h"

class SoftwareDisplay : public IDisplay {
    public:
        SoftwareDisplay(IAccumulator* accumulator,
                    IMotorController* motorController, 
                    IPedal* gasPedal, IPedal* brakePedal) {
            setComponentType(COMPONENT_DISPLAY);
            setObjectType(OBJECT_SOFTWARE);

            _accumulator = accumulator;
            _motorController = motorController;
            _gasPedal = gasPedal;
            _brakePedal = brakePedal;

        }

        SoftwareDisplay(IAccumulator* accumulator,
                    IMotorController* motorController, 
                    IPedal* gasPedal, IPedal* brakePedal, id_sub_component_t componentSubId)
            : SoftwareDisplay(accumulator, motorController, gasPedal, brakePedal) {
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

            /**
             * @brief 
             * 
             * Send Cell Values
             * 
             */

            uint16_t cmdId = 0;

            subMessage.length = 5;
            subMessage.data[0] = cmdId;
            subMessage.data[1] = _accumulator->getMinVoltage();
            subMessage.data[2] = _accumulator->getMaxVoltage();
            subMessage.data[3] = _accumulator->getMinTemp();
            subMessage.data[4] = _accumulator->getMaxTemp();

            carMessage.addSubMessage(subMessage);
            
            /**
             * @brief 
             * 
             * Send MotorController Values 1/2
             * 
             */
            
/*
            cmdId++;

            float speedFloat = _motorController->getSpeed();
            uint16_t speed = ((float)speedFloat * 65535);
            
            float currentFloat = _motorController->getCurrent();
            uint16_t current = ((float)currentFloat * 65535);

            subMessage.data[0] = cmdId;
            subMessage.data[1] = speed & 0xFF;          
            subMessage.data[2] = (speed >> 8) & 0xFF;     
            subMessage.data[3] = current & 0xFF;          
            subMessage.data[4] = (current >> 8) & 0xFF;       

            carMessage.addSubMessage(subMessage);

*/

             /**
             * @brief 
             * 
             * Send MotorController Values 2/2
             * 
             */
            /*

            cmdId++;

            float dcVoltageFloat = _motorController->getDcVoltage();
            uint16_t dcVoltage = ((float)dcVoltageFloat * 65535);

            subMessage.data[0] = cmdId;
            subMessage.data[1] = _motorController->getMotorTemp();
            subMessage.data[2] = _motorController->getAirTemp();
            subMessage.data[3] = dcVoltage & 0xFF;          
            subMessage.data[4] = (dcVoltage >> 8) & 0xFF; 

            carMessage.addSubMessage(subMessage);
*/
            
            /*
                subMessage.data[3] = _motorController->getRFE();
                subMessage.data[4] = _motorController->getRUN();
            */
            
             /**
             * @brief 
             * 
             * Send Pedal Values
             * 
             
            

            cmdId++;

            subMessage.data[0] = cmdId;
            subMessage.data[1] = _gasPedal->getValue();
            subMessage.data[2] = _motorController->getDcVoltage();
            subMessage.data[3] = _motorController->getRFE();
            subMessage.data[4] = _motorController->getRUN();

            carMessage.addSubMessage(subMessage);

            */

            #ifdef PEDAL_MESSAGE_HANDLER_DEBUG
                            pcSerial.printf("[SoftwarePedal]@buildMessage: SoftwareObject calibrationStatus: 0x%x\tmsg.data[0]: 0x%x\tvalueToSent: %i\n", calibrationStatus, subMessage.data[0], valueToSent);
                        #endif


            return MESSAGE_BUILD_OK;
        }

        virtual message_parse_result_t parseMessage(CarMessage &carMessage) {
            message_parse_result_t result = MESSAGE_PARSE_OK;
            for (car_sub_message_t &subMessage : carMessage.subMessages) {
                #ifdef LIVE_MIN_TEMP
                    if(subMessage.length != 4) // not a valid message
                    result = MESSAGE_PARSE_ERROR;

                    this->setMaxCellTemperature(subMessage.data[3]);
                #else
                    if(subMessage.length != 3) // not a valid message
                    result = MESSAGE_PARSE_ERROR;
                #endif
            }
            
            return result;
        }

    private:

        accumulator_status_t _status = 0;
        
        IAccumulator* _accumulator;
        IMotorController* _motorController;
        IPedal* _gasPedal;
        IPedal* _brakePedal;

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