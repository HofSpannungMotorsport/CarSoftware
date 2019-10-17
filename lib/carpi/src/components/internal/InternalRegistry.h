#ifndef INTERNALREGISTRY_H
#define INTERNALREGISTRY_H

#include "../interface/IRegistry.h"

/*
    RAM Based Registry (including Syncing accross multiple Microcontrollers)
*/

/*
    CarMessage Layout (only subMessage)

    data: 0           1   2   3     4     5     6     7
          Data Type   Index   Data (LSB) ... Data (MSB)

    Data Type = float, uint8_t ... bool, registryReady (true), registryReady(false)
    Index = index of the data (not for registryReady, it has only the Data Type set)
    Data = between 1 and 4 bit data, starting with the least segnificant byte
*/

class InternalRegistry : public IRegistry {
    public:
        InternalRegistry(id_sub_component_t componentSubId) {
            setComponentSubId(componentSubId);

            // Initializing all values in all arrays with 0

            for (registry_index_t i = 0; i < float_registry_size; i++) 
                _floatRegistry[i] = 0;
            
            for (registry_index_t i = 0; i < uint8_registry_size; i++)
                _uint8Registry[i] = 0;
            
            for (registry_index_t i = 0; i < uint16_registry_size; i++)
                _uint16Registry[i] = 0;

            for (registry_index_t i = 0; i < uint32_registry_size; i++)
                _uint32Registry[i] = 0;
            
            for (registry_index_t i = 0; i < int8_registry_size; i++)
                _int8Registry[i] = 0;
            
            for (registry_index_t i = 0; i < int16_registry_size; i++)
                _int16Registry[i] = 0;
            
            for (registry_index_t i = 0; i < int32_registry_size; i++)
                _int32Registry[i] = 0;
            
            for (registry_index_t i = 0; i < bool_registry_size; i++)
                _boolRegistry[i] = false;
        }

        bool getReady() {
            return _ready;
        }

        void setReady(bool ready) {
            _ready = ready;

            if (_ready)
                _sendCommand(REGISTRY_SET_READY, SEND_PRIORITY_REGISTRY, IS_NOT_DROPABLE);
            else
                _sendCommand(REGISTRY_SET_NOT_READY, SEND_PRIORITY_REGISTRY, IS_NOT_DROPABLE);
        }

        // Getter
        // float
        float getFloat(float_registry_index_t index) {
            if (index < float_registry_size)
                return _floatRegistry[index];
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;

            return 0;
        }
        
        // int
        uint8_t getUInt8(uint8_registry_index_t index) {
            if (index < uint8_registry_size)
                return _uint8Registry[index];
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;

            return 0;
        }

        uint16_t getUInt16(uint16_registry_index_t index) {
            if (index < uint16_registry_size)
                return _uint16Registry[index];
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;

            return 0;
        }

        uint32_t getUInt32(uint32_registry_index_t index) {
            if (index < uint32_registry_size)
                return _uint32Registry[index];
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;

            return 0;
        }

        int8_t getInt8(int8_registry_index_t index) {
            if (index < int8_registry_size)
                return _int8Registry[index];
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;

            return 0;
        }

        int16_t getInt16(int16_registry_index_t index) {
            if (index < int16_registry_size)
                return _int16Registry[index];
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;

            return 0;
        }

        int32_t getInt32(int32_registry_index_t index) {
            if (index < int32_registry_size)
                return _int32Registry[index];
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;

            return 0;
        }

        // bool
        bool getBool(bool_registry_index_t index) {
            if (index < bool_registry_size)
                return _boolRegistry[index];
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;

            return false;
        }


        // Setter
        // float
        void setFloat(float_registry_index_t index, float value) {
            if (_setData<float, float_registry_index_t>(index, value, _floatRegistry, float_registry_size))
                _sendFloat(index, value);
        }
        
        // int
        void setUInt8(uint8_registry_index_t index, uint8_t value) {
            if (_setData<uint8_t, uint8_registry_index_t>(index, value, _uint8Registry, uint8_registry_size))
                _sendData<uint8_t, uint8_registry_index_t>(index, value, REGISTRY_TYPE_UINT8);
        }

        void setUInt16(uint16_registry_index_t index, uint16_t value) {
            if (_setData<uint16_t, uint16_registry_index_t>(index, value, _uint16Registry, uint16_registry_size))
                _sendData<uint16_t, uint16_registry_index_t>(index, value, REGISTRY_TYPE_UINT16);
        }

        void setUInt32(uint32_registry_index_t index, uint32_t value)  {
            if (_setData<uint32_t, uint32_registry_index_t>(index, value, _uint32Registry, uint32_registry_size))
                _sendData<uint32_t, uint32_registry_index_t>(index, value, REGISTRY_TYPE_UINT32);
        }

        void setInt8(int8_registry_index_t index, int8_t value) {
            if (_setData<int8_t, int8_registry_index_t>(index, value, _int8Registry, int8_registry_size))
                _sendData<int8_t, int8_registry_index_t>(index, value, REGISTRY_TYPE_INT8);
        }

        void setInt16(int16_registry_index_t index, int16_t value) {
            if (_setData<int16_t, int16_registry_index_t>(index, value, _int16Registry, int16_registry_size))
                _sendData<int16_t, int16_registry_index_t>(index, value, REGISTRY_TYPE_INT16);
        }

        void setInt32(int32_registry_index_t index, int32_t value) {
            if (_setData<int32_t, int32_registry_index_t>(index, value, _int32Registry, int32_registry_size))
                _sendData<int32_t, int32_registry_index_t>(index, value, REGISTRY_TYPE_INT32);
        }

        // bool
        void setBool(bool_registry_index_t index, bool value) {
            if (_setData<bool, bool_registry_index_t>(index, value, _boolRegistry, bool_registry_size))
                _sendData<bool, bool_registry_index_t>(index, value, REGISTRY_TYPE_BOOL);
        }


        virtual status_t getStatus() {
            return _status;
        }

        virtual void setStatus(status_t status) {
            // No implementation needed
        }

        // Receive a CarMessage and save the got value in the right register
        void receive(CarMessage &carMessage) {
            for (car_sub_message_t &subMessage : carMessage.subMessages) {
                switch(subMessage.data[0]) {
                    case REGISTRY_SET_NOT_READY:
                        _ready = false;
                        break;
                    
                    case REGISTRY_SET_READY:
                        _ready = true;
                        break;
                    
                    case REGISTRY_TYPE_FLOAT:
                        _receiveFloat(subMessage);
                        break;
                    
                    case REGISTRY_TYPE_UINT8:
                        _receiveData<uint8_t, uint8_registry_index_t>(subMessage, _uint8Registry, uint8_registry_size);
                        break;
                    
                    case REGISTRY_TYPE_UINT16:
                        _receiveData<uint16_t, uint16_registry_index_t>(subMessage, _uint16Registry, uint16_registry_size);
                        break;
                    
                    case REGISTRY_TYPE_UINT32:
                        _receiveData<uint32_t, uint32_registry_index_t>(subMessage, _uint32Registry, uint32_registry_size);
                        break;
                    
                    case REGISTRY_TYPE_INT8:
                        _receiveData<int8_t, int8_registry_index_t>(subMessage, _int8Registry, int8_registry_size);
                        break;
                    
                    case REGISTRY_TYPE_INT16:
                        _receiveData<int16_t, int16_registry_index_t>(subMessage, _int16Registry, int16_registry_size);
                        break;
                    
                    case REGISTRY_TYPE_INT32:
                        _receiveData<int32_t, int32_registry_index_t>(subMessage, _int32Registry, int32_registry_size);
                        break;
                    
                    case REGISTRY_TYPE_BOOL:
                        _receiveData<bool, bool_registry_index_t>(subMessage, _boolRegistry, bool_registry_size);
                        break;

                    default:
                        _status |= REGISTRY_ERROR_UNKNOWN_MSG_RECEIVED;
                        break;
                }
            }
        }
    
    private:
        status_t _status = 0;
        bool _ready = false;

        // float
        float _floatRegistry[float_registry_size];
        
        // int
        uint8_t _uint8Registry[uint8_registry_size];
        uint16_t _uint16Registry[uint16_registry_size];
        uint32_t _uint32Registry[uint32_registry_size];
        int8_t _int8Registry[int8_registry_size];
        int16_t _int16Registry[int16_registry_size];
        int32_t _int32Registry[int32_registry_size];

        // bool
        bool _boolRegistry[bool_registry_size];


        // Private Setter
        template<typename T, typename registry_index_type_t>
        bool _setData(registry_index_type_t index, T value, T typeRegistry[], registry_index_t indexSize) {
            if (index < indexSize) {
                typeRegistry[index] = value;
                return true;
            } // else
            _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;
            return false;
        }


        // Helper
        registry_index_t _getIndex(car_sub_message_t &subMessage) {
            uint16_t index = 0;
            index |= subMessage.data[1] & 0xFF;
            index |= (subMessage.data[2] << 8) & 0xFF00;

            return index;
        }


        // Private Send Methods
        // float (special)
        void _sendFloat(float_registry_index_t index, float value) {
            // Get disassambled float
            uint8_t disassambledFloat[4];
            _disassambleFloat(value, disassambledFloat);

            // Make uint8 value Array
            uint16_t index0 = index & 0xFF;
            uint16_t index1 = (index >> 8) & 0xFF;
            uint8_t data[6] = {(uint8_t)index0, (uint8_t)index1};
            // Copy float
            memcpy(&data[2], disassambledFloat, 4);

            _sendCommand(REGISTRY_TYPE_FLOAT, data, 6, SEND_PRIORITY_REGISTRY, IS_NOT_DROPABLE);
        }

        void _receiveFloat(car_sub_message_t &subMessage) {
            if (subMessage.length != 7) {
                _status |= REGISTRY_ERROR_RECEIVED_TOO_LONG_MSG;
                return;
            }
            
            registry_index_t index = _getIndex(subMessage);

            uint8_t disassambledFloat[4];
            memcpy(disassambledFloat, &subMessage.data[3], 4);

            float value = _reassambleFloat(disassambledFloat);

            _setData<float, float_registry_index_t>((float_registry_index_t)index, value, _floatRegistry, float_registry_size);
        }

        // for int and bool, we can use a template
        template<typename T, typename registry_index_type_t>
        void _sendData(registry_index_type_t index, T value, registry_message_command_t messageType) {
            uint8_t typeSize = sizeof(T);
            uint8_t dataSize = typeSize + 2;

            uint16_t index0 = index & 0xFF;
            uint16_t index1 = (index >> 8) & 0xFF;
            uint8_t data[dataSize] = {(uint8_t)index0, (uint8_t)index1};

            uint32_t value32 = value;
            
            for (uint8_t i = 0; i < typeSize; i++) {
                uint32_t valueCut = (value32 >> (8 * i)) & 0xFF;
                data[2 + i] = (uint8_t)valueCut;
            }

            _sendCommand(messageType, data, dataSize, SEND_PRIORITY_REGISTRY, IS_NOT_DROPABLE);
        }

        template<typename T, typename registry_index_type_t>
        void _receiveData(car_sub_message_t &subMessage, T typeRegistry[], registry_index_t indexSize) {
            uint8_t typeSize = sizeof(T);

            if (subMessage.length != typeSize + 3) {
                _status |= REGISTRY_ERROR_RECEIVED_TOO_LONG_MSG;
                return;
            }

            registry_index_t index = _getIndex(subMessage);
            uint32_t value32 = 0;

            for (uint8_t i = 0; i < typeSize; i++) {
                value32 |= ((uint32_t)subMessage.data[3 + i] & 0xFF) << (8 * i);
            }

            T value = value32;

            _setData<T, registry_index_type_t>((registry_index_type_t)index, value, typeRegistry, indexSize);
        }
};

#endif // INTERNALREGISTRY_H