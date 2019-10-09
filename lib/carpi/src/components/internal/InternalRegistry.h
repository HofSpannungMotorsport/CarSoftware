#ifndef INTERNALREGISTRY_H
#define INTERNALREGISTRY_H

#include "../interface/IRegistry.h"

class InternalRegistry : public IRegistry {
    public:
        InternalRegistry(id_sub_component_t componentSubId) {
            setComponentSubId(componentSubId);

            // Initializing all values in all arrays with 0

            for (registry_index_t i = 0; i < float_registry_size; i++) 
                floatRegistry[i] = 0;
            
            for (registry_index_t i = 0; i < uint8_registry_size; i++)
                uint8Registry[i] = 0;
            
            for (registry_index_t i = 0; i < uint16_registry_size; i++)
                uint16Registry[i] = 0;

            for (registry_index_t i = 0; i < uint32_registry_size; i++)
                uint32Registry[i] = 0;
            
            for (registry_index_t i = 0; i < int8_registry_size; i++)
                int8Registry[i] = 0;
            
            for (registry_index_t i = 0; i < int16_registry_size; i++)
                int16Registry[i] = 0;
            
            for (registry_index_t i = 0; i < int32_registry_size; i++)
                int32Registry[i] = 0;
            
            for (registry_index_t i = 0; i < bool_registry_size; i++)
                boolRegistry[i] = false;
        }

        // Getter
        // float
        virtual float getFloat(float_registry_index_t index) {
            if (index < float_registry_size)
                return floatRegistry[index];
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;

            return 0;
        }
        
        // int
        virtual uint8_t getUInt8(uint8_registry_index_t index) {
            if (index < uint8_registry_size)
                return uint8Registry[index];
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;

            return 0;
        }

        virtual uint16_t getUInt16(uint16_registry_index_t index) {
            if (index < uint16_registry_size)
                return uint16Registry[index];
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;

            return 0;
        }

        virtual uint32_t getUInt32(uint32_registry_index_t index) {
            if (index < uint32_registry_size)
                return uint32Registry[index];
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;

            return 0;
        }

        virtual int8_t getInt8(int8_registry_index_t index) {
            if (index < int8_registry_size)
                return int8Registry[index];
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;

            return 0;
        }

        virtual int16_t getInt16(int16_registry_index_t index) {
            if (index < int16_registry_size)
                return int16Registry[index];
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;

            return 0;
        }

        virtual int32_t getInt32(int32_registry_index_t index) {
            if (index < int32_registry_size)
                return int32Registry[index];
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;

            return 0;
        }

        // bool
        virtual bool getBool(bool_registry_index_t index) {
            if (index < bool_registry_size)
                return boolRegistry[index];
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;

            return 0;
        }


        // Setter
        // float
        virtual void setFloat(float_registry_index_t index, float value) {
            if (index < float_registry_size)
                floatRegistry[index] = value;
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;
        }
        
        // int
        virtual void setUInt8(uint8_registry_index_t index, uint8_t value) {
            if (index < uint8_registry_size)
                uint8Registry[index] = value;
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;
        }

        virtual void setUInt16(uint16_registry_index_t index, uint16_t value) {
            if (index < uint16_registry_size)
                uint16Registry[index] = value;
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;
        }

        virtual void setUInt32(uint32_registry_index_t index, uint32_t value)  {
            if (index < uint32_registry_size)
                uint32Registry[index] = value;
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;
        }

        virtual void setInt8(int8_registry_index_t index, int8_t value) {
            if (index < int8_registry_size)
                int8Registry[index] = value;
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;
        }

        virtual void setInt16(int16_registry_index_t index, int16_t value) {
            if (index < int16_registry_size)
                int16Registry[index] = value;
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;
        }

        virtual void setInt32(int32_registry_index_t index, int32_t value) {
            if (index < int32_registry_size)
                int32Registry[index] = value;
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;
        }

        // bool
        virtual void setBool(bool_registry_index_t index, bool value) {
            if (index < bool_registry_size)
                boolRegistry[index] = value;
            else
                _status |= REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS;
        }


        virtual status_t getStatus() {
            return _status;
        }

        void setStatus(status_t status) {
            // No implementation needed
        }

        void receive(CarMessage &carMessage) {

        }
    
    private:
        status_t _status = 0;

        // float
        float floatRegistry[float_registry_size];
        
        // int
        uint8_t uint8Registry[uint8_registry_size];
        uint16_t uint16Registry[uint16_registry_size];
        uint32_t uint32Registry[uint32_registry_size];
        int8_t int8Registry[int8_registry_size];
        int16_t int16Registry[int16_registry_size];
        int32_t int32Registry[int32_registry_size];

        // bool
        bool boolRegistry[bool_registry_size];
};

#endif // INTERNALREGISTRY_H