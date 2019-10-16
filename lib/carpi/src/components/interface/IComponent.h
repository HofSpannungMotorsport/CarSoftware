#ifndef ICOMPONENT_H
#define ICOMPONENT_H

#include "communication/componentIds.h"
#include "ILogable.h"

typedef uint8_t component_object_type_datatype_t;
enum component_object_type_t : component_object_type_datatype_t {
    OBJECT_UNDEFINED = 0x0,
    OBJECT_HARDWARE = 0x1,
    OBJECT_SOFTWARE = 0x2,
    OBJECT_INTERNAL = 0x3,
    OBJECT_RUNABLE = 0x4
};

class IComponent : public ILogable {
    public:
        IComponent() {}
        IComponent(id_sub_component_t componentSubId)
            : _componentSubId(componentSubId) {}

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

#endif // IComponent_H