#ifndef IID_H
#define IID_H

#include "communication/componentIds.h"

typedef uint8_t component_object_type_datatype_t;
enum component_object_type_t : component_object_type_datatype_t {
    OBJECT_UNDEFINED = 0x0,
    OBJECT_HARDWARE = 0x1,
    OBJECT_SOFTWARE = 0x2
};

class IID {
    public:
        IID() {}
        IID(can_component_t componentId)
            : _componentId(componentId) {}

        virtual void setComponentSubId(id_sub_component_t componentSubId) {
            _componentSubId = componentSubId;
        }

        virtual id_component_type_t getComponentType() {
            return _componentType;
        }

        virtual id_sub_component_t getComponentSubId() {
            return _componentSubId;
        }

        virtual id_component_t getComponentId() {
            return componentId::getComponentId(_componentType, _componentSubId);
        }

        virtual component_object_type_t getObjectType() {
            return _objectType;
        }

    protected:
        virtual void setComponentType(id_component_type_t componentType) {
            _componentType = componentType;
        }

        virtual void setObjectType(component_object_type_t objectType) {
            _objectType = objectType;
        }
    
    private:
        id_component_type_t _componentType;
        id_sub_component_t _componentSubId;
        component_object_type_t _objectType = OBJECT_UNDEFINED;
};

#endif // IID_H