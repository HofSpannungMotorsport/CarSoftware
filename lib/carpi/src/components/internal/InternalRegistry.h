#ifndef INTERNALREGISTRY_H
#define INTERNALREGISTRY_H

#include "../interface/IRegistry.h"
#include "crc16.h"

/*
    RAM Based Registry (including Syncing accross multiple Microcontrollers)
*/

/*
    CarMessage Layout

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
                _sendCommand(REGISTRY_SET_READY);
            else
                _sendCommand(REGISTRY_SET_NOT_READY);
        }

        // CRC
        uint16_t getCrc() {
            uint16_t crc;

            crc = _getCrc<float>(_floatRegistry, float_registry_size);
            crc = _updateCrc<uint8_t>(crc, _uint8Registry, uint8_registry_size);
            crc = _updateCrc<uint16_t>(crc, _uint16Registry, uint16_registry_size);
            crc = _updateCrc<uint32_t>(crc, _uint32Registry, uint32_registry_size);
            crc = _updateCrc<int8_t>(crc, _int8Registry, int8_registry_size);
            crc = _updateCrc<int16_t>(crc, _int16Registry, int16_registry_size);
            crc = _updateCrc<int32_t>(crc, _int32Registry, int32_registry_size);

            return crc;
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
            switch(carMessage[0]) {
                case REGISTRY_SET_NOT_READY:
                    _ready = false;
                    break;

                case REGISTRY_SET_READY:
                    _ready = true;
                    break;

                case REGISTRY_CRC: {
                        uint16_t thatCrc = 0 || ((carMessage[1] >> 8) && 0xFF) || (carMessage[2] && 0xFF);
                        if (getCrc() != thatCrc)
                            _sendCommand(REGISTRY_CRC_NOT_MATCHING);
                    }
                    break;

                case REGISTRY_CRC_NOT_MATCHING:
                    _reSync();
                    break;

                case REGISTRY_TYPE_FLOAT:
                    _receiveFloat(carMessage);
                    break;

                case REGISTRY_TYPE_UINT8:
                    _receiveData<uint8_t, uint8_registry_index_t>(carMessage, _uint8Registry, uint8_registry_size);
                    break;

                case REGISTRY_TYPE_UINT16:
                    _receiveData<uint16_t, uint16_registry_index_t>(carMessage, _uint16Registry, uint16_registry_size);
                    break;

                case REGISTRY_TYPE_UINT32:
                    _receiveData<uint32_t, uint32_registry_index_t>(carMessage, _uint32Registry, uint32_registry_size);
                    break;

                case REGISTRY_TYPE_INT8:
                    _receiveData<int8_t, int8_registry_index_t>(carMessage, _int8Registry, int8_registry_size);
                    break;

                case REGISTRY_TYPE_INT16:
                    _receiveData<int16_t, int16_registry_index_t>(carMessage, _int16Registry, int16_registry_size);
                    break;

                case REGISTRY_TYPE_INT32:
                    _receiveData<int32_t, int32_registry_index_t>(carMessage, _int32Registry, int32_registry_size);
                    break;

                case REGISTRY_TYPE_BOOL:
                    _receiveData<bool, bool_registry_index_t>(carMessage, _boolRegistry, bool_registry_size);
                    break;

                default:
                    _status |= REGISTRY_ERROR_UNKNOWN_MSG_RECEIVED;
                    break;
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


        // Private Getter
        template<typename registry_type_t>
        uint16_t _getCrc(registry_type_t *registry, registry_index_t elementCount) {
            uint16_t crc;
            bool afterAddZero = false;

            if (sizeof(registry_type_t) <= 1 && elementCount % 2) {
                --elementCount;
                afterAddZero = true;
            }

            crc = crc_16<registry_index_t>((unsigned char*)registry, elementCount * sizeof(registry_type_t) / 2);

            if (afterAddZero) { // Only called if datatype is 1 byte long AND has uneven element count
                uint16_t tempVal = 0 || ((uint8_t*)registry)[elementCount - 1];
                crc = update_crc_16(crc, tempVal);
            }

            return crc;
        }

        template<typename registry_type_t>
        uint16_t _updateCrc(uint16_t crc, registry_type_t *registry, registry_index_t elementCount) {
            bool afterAddZero = false;

            if (sizeof(registry_type_t) <= 1 && elementCount % 2) {
                --elementCount;
                afterAddZero = true;
            }

            crc = batch_update_crc_16<registry_index_t>(crc, (unsigned char*)registry, elementCount * sizeof(registry_type_t) / 2);

            if (afterAddZero) {
                uint16_t tempVal = 0 || ((uint8_t*)registry)[elementCount - 1];
                crc = update_crc_16(crc, tempVal);
            }

            return crc;
        }

        void _reSync() {
            // Resync special case float
            _reSyncFloat();

            // Resync everything else
            _reSyncType<uint8_t, uint8_registry_index_t>(_uint8Registry, uint8_registry_size, REGISTRY_TYPE_UINT8);
            _reSyncType<uint16_t, uint16_registry_index_t>(_uint16Registry, uint16_registry_size, REGISTRY_TYPE_UINT16);
            _reSyncType<uint32_t, uint32_registry_index_t>(_uint32Registry, uint32_registry_size, REGISTRY_TYPE_UINT32);

            _reSyncType<int8_t, int8_registry_index_t>(_int8Registry, int8_registry_size, REGISTRY_TYPE_INT8);
            _reSyncType<int16_t, int16_registry_index_t>(_int16Registry, int16_registry_size, REGISTRY_TYPE_INT16);
            _reSyncType<int32_t, int32_registry_index_t>(_int32Registry, int32_registry_size, REGISTRY_TYPE_INT32);

            _reSyncType<bool, bool_registry_index_t>(_boolRegistry, bool_registry_size, REGISTRY_TYPE_BOOL);
        }

        void _reSyncFloat() {
            for (registry_index_t i = 0; i < float_registry_size; i++) {
                _sendFloat((float_registry_index_t)i, _floatRegistry[i]);
            }
        }

        template<typename T, typename registry_index_type_t>
        void _reSyncType(T *registry, registry_index_t elementCount, registry_message_command_t messageType) {
            for (registry_index_t i = 0; i < elementCount; i++) {
                _sendData<T, registry_index_type_t>((registry_index_type_t)i, registry[i], messageType);
            }
        }


        // Private Setter
        template<typename T, typename registry_index_type_t>
        bool _setData(registry_index_type_t index, T value, T typeRegistry[], registry_index_t indexSize) {
            if (index < indexSize) {
                if (typeRegistry[index] == value) 
                    return false;

                typeRegistry[index] = value;
                return true;
            } // else
            _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;
            return false;
        }


        // Helper
        registry_index_t _getIndex(CarMessage &carMessage) {
            uint16_t index = 0;
            index |= carMessage[1] & 0xFF;
            index |= (carMessage[2] << 8) & 0xFF00;

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
            memCpy<uint8_t>(&data[2], disassambledFloat, 4);

            _sendCommand(REGISTRY_TYPE_FLOAT, data, 6);
        }

        void _receiveFloat(CarMessage &carMessage) {
            if (carMessage.getLength() != 7) {
                _status |= REGISTRY_ERROR_RECEIVED_TOO_LONG_MSG;
                return;
            }
            
            registry_index_t index = _getIndex(carMessage);

            uint8_t disassambledFloat[4];
            memCpy<uint8_t>(disassambledFloat, &carMessage[3], 4);

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

            _sendCommand(messageType, data, dataSize);
        }

        template<typename T, typename registry_index_type_t>
        void _receiveData(CarMessage &carMessage, T typeRegistry[], registry_index_t indexSize) {
            uint8_t typeSize = sizeof(T);

            if (carMessage.getLength() != typeSize + 3) {
                _status |= REGISTRY_ERROR_RECEIVED_TOO_LONG_MSG;
                return;
            }

            registry_index_t index = _getIndex(carMessage);
            uint32_t value32 = 0;

            for (uint8_t i = 0; i < typeSize; i++) {
                value32 |= ((uint32_t)carMessage[3 + i] & 0xFF) << (8 * i);
            }

            T value = value32;

            _setData<T, registry_index_type_t>((registry_index_type_t)index, value, typeRegistry, indexSize);
        }
};

#endif // INTERNALREGISTRY_H