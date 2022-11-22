#ifndef HARDWAREACCUMULATOR_H
#define HARDWAREACCUMULATOR_H

#include "../interface/IAccumulator.h"

class HardwareAccumulator : public IAccumulator {
    public:
        HardwareAccumulator(){
            setComponentType(COMPONENT_ACCUMULATOR);
            setObjectType(OBJECT_HARDWARE);
        }

        HardwareAccumulator(id_sub_component_t componentSubId){
                setComponentSubId(componentSubId);
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

        virtual message_build_result_t buildMessage(CarMessage &carMessage) {
            car_sub_message_t subMessage;

            #ifdef LIVE_MIN_TEMP
                subMessage.length = 4;
                subMessage.data[3] = this->getMinTemp();
            #else
                subMessage.length = 3;
            #endif

            subMessage.data[0] = this->getMinVoltage();
            subMessage.data[1] = this->getMaxVoltage();
            subMessage.data[2] = this->getMaxTemp();

            // change line below when type of pedal_value_t changes
            carMessage.addSubMessage(subMessage);

            return MESSAGE_BUILD_OK;
        }

        virtual message_parse_result_t parseMessage(CarMessage &carMessage) {
            message_parse_result_t result = MESSAGE_PARSE_OK;
            for (car_sub_message_t &subMessage : carMessage.subMessages) {
                /**
                 * No function
                 * 
                 */
            }
            
            return result;
        }

    protected:
        accumulator_status_t _status = 0;
        
        uint16_t _minCellVoltage = 0;
        uint16_t _maxCellVoltage = 0;
        uint16_t _minCellTemperature = 0;
        uint16_t _maxCellTemperature = 0;
};

#endif