#ifndef IREGISTRY_H
#define IREGISTRY_H

#include "communication/SelfSyncable.h"

typedef uint16_t registry_index_t;

enum float_registry_index_t : registry_index_t {

};
const registry_index_t float_registry_size = 0;

enum uint8_registry_index_t : registry_index_t {

};
const registry_index_t uint8_registry_size = 0;

enum uint16_registry_index_t : registry_index_t {

};
const registry_index_t uint16_registry_size = 0;

enum uint32_registry_index_t : registry_index_t {

};
const registry_index_t uint32_registry_size = 0;

enum int8_registry_index_t : registry_index_t {

};
const registry_index_t int8_registry_size = 0;

enum int16_registry_index_t : registry_index_t {

};
const registry_index_t int16_registry_size = 0;

enum int32_registry_index_t : registry_index_t {

};
const registry_index_t int32_registry_size = 0;

enum bool_registry_index_t : registry_index_t {

};
const registry_index_t bool_registry_size = 0;


enum registry_error_type_t : status_t {
    REGISTRY_ERROR_ARRAY_OUT_OF_BOUNDS = 0x1
};


class IRegistry : public SelfSyncable {
    public:
        IRegistry() {
            setComponentType(COMPONENT_SYSTEM);
            setObjectType(OBJECT_INTERNAL);
        }

        // Getter
        // float
        virtual float getFloat(float_registry_index_t index) = 0;
        
        // int
        virtual uint8_t getUInt8(uint8_registry_index_t index) = 0;
        virtual uint16_t getUInt16(uint16_registry_index_t index) = 0;
        virtual uint32_t getUInt32(uint32_registry_index_t index) = 0;
        virtual int8_t getInt8(int8_registry_index_t index) = 0;
        virtual int16_t getInt16(int16_registry_index_t index) = 0;
        virtual int32_t getInt32(int32_registry_index_t index) = 0;

        // bool
        virtual bool getBool(bool_registry_index_t index) = 0;


        // Setter
        // float
        virtual void setFloat(float_registry_index_t index, float value) = 0;
        
        // int
        virtual void setUInt8(uint8_registry_index_t index, uint8_t value) = 0;
        virtual void setUInt16(uint16_registry_index_t index, uint16_t value) = 0;
        virtual void setUInt32(uint32_registry_index_t index, uint32_t value) = 0;
        virtual void setInt8(int8_registry_index_t index, int8_t value) = 0;
        virtual void setInt16(int16_registry_index_t index, int16_t value) = 0;
        virtual void setInt32(int32_registry_index_t index, int32_t value) = 0;

        // bool
        virtual void setBool(bool_registry_index_t index, bool value) = 0;
};

#endif // IREGISTRY_H