#ifndef SOFTWAREACCUMULATOR_H
#define SOFTWAREACCUMULATOR_H

#include "../interface/IAccumulator.h"

class SoftwareAccumulator : public IAccumulator {
    public:
        SoftwareAccumulator() {
            setComponentType(COMPONENT_ACCUMULATOR);
            setObjectType(OBJECT_SOFTWARE);
        }

        SoftwareAccumulator(id_sub_component_t componentSubId)
            : SoftwareAccumulator() {
            setComponentSubId(componentSubId);
        }

        virtual message_build_result_t buildMessage(CarMessage &carMessage) {
            /*
            car_sub_message_t subMessage;

            
            subMessage.length = 1;

            #ifdef PEDAL_MESSAGE_HANDLER_DEBUG
                            pcSerial.printf("[SoftwarePedal]@buildMessage: SoftwareObject calibrationStatus: 0x%x\tmsg.data[0]: 0x%x\tvalueToSent: %i\n", calibrationStatus, subMessage.data[0], valueToSent);
                        #endif

                        carMessage.addSubMessage(subMessage);


            */
            
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


                this->setMinCellVoltage(subMessage.data[0]);
                this->setMaxCellVoltage(subMessage.data[1]);
                this->setMaxCellTemperature(subMessage.data[2]);

                #ifdef ACCUMULATOR_MESSAGE_HANDLER_DEBUG
                    //pcSerial.printf("[SoftwareAccumulator]@parseMessage: HardwareObject ");
                #endif

            }
            
            return result;
        }

        virtual accumulator_status_t getStatus() {
            return _status;
        }

        uint16_t getMinVoltage(){
            return _minCellVoltage;
        }

        uint16_t getMaxVoltage(){
            return _maxCellVoltage;
        }

        uint16_t getMinTemp(){
            return _minCellTemperature;
        }

        uint16_t getMaxTemp(){
            return _maxCellTemperature;
        }



    private:

        accumulator_status_t _status = 0;
        
        uint16_t _minCellVoltage = 0;
        uint16_t _maxCellVoltage = 0;
        uint16_t _minCellTemperature = 0;
        uint16_t _maxCellTemperature = 0;

        virtual void setStatus(accumulator_status_t status) {
            _status = status;
        }
        
        virtual void setMinCellVoltage(uint16_t minCellVoltage) {
            _minCellVoltage = minCellVoltage;
        }

        virtual void setMaxCellVoltage(uint16_t maxCellVoltage) {
            _maxCellVoltage = maxCellVoltage;
        }

        virtual void setMinCellTemperature(uint16_t minCellTemperature) {
            _minCellTemperature = minCellTemperature;
        }

        virtual void setMaxCellTemperature(uint16_t maxCellTemperature) {
            _maxCellTemperature = maxCellTemperature;
        }

};

#endif