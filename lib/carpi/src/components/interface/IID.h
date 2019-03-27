#ifndef IID_H
#define IID_H

#include "communication/can_ids.h"

class IID {
    public:
        IID() {}
        IID(can_component_t componentId)
            : _componentId(componentId) {}

        virtual void setComponentId(can_component_t componentId) {
            _componentId = componentId;
        }

        /*  Should be set directly by the object
        virtual void setTelegramTypeId(can_telegram_type_t telegramTypeId) {
            _telegramTypeId = telegramTypeId;
        }

        virtual void setObjectType(can_object_type_t objectType) {
            _objectType = objectType;
        }
        */

        virtual can_component_t getComponentId() {
            return _componentId;
        }

        virtual can_telegram_type_t getTelegramTypeId() {
            return _telegramTypeId;
        }

        virtual can_object_type_t getObjectType() {
            return _objectType;
        }
    
    protected:
        can_component_t _componentId;
        can_telegram_type_t _telegramTypeId;
        can_object_type_t _objectType;
};

#endif // IID_H