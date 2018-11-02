#ifndef IID_H
#define IID_H

#include <stdint.h>
#include "../../can/can_ids.h"

class IID {
    public:
        IID() {}
        IID(can_component_t componentId)
            : _componentId(componentId) {}

        virtual void setComponentId(can_component_t componentId) {
            _componentId = componentId;
        }

        virtual can_component_t getComponentId() {
            return _componentId;
        }

        virtual can_telegram_type_t getTelegramTypeId() {
            return _telegramTypeId;
        }
    
    private:
        can_component_t _componentId;
        can_telegram_type_t _telegramTypeId;
};

#endif // IID_H