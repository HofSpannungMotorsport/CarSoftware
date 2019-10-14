#ifndef ISDCARD_H
#define ISDCARD_H

#include <string>
using namespace std;

#include "IComponent.h"

#define SD_CARD_SD_LOG_COUNT 2
enum sd_log_id_sd_card_t : sd_log_id_t {
    SD_LOG_ID_BEGIN_FILE = 0,
    SD_LOG_ID_CARPI_VERSION = 1,
    SD_LOG_ID_SCAR_STATE = 2,
    SD_LOG_ID_SCAR_ERROR = 3
};

enum sd_error_type_t : status_t {
    SD_OK =                     0x0,
    SD_INITALIZATION_FAILED =   0x1,
    SD_FILE_ACCESS_FAILED =     0x2,
    SD_NOT_INITIALIZED_BEFORE = 0x4,
    SD_FILE_NOT_OPENED_BEFORE = 0x8
};

class ISDCard : public IComponent {
    public:
        ISDCard() {
            setComponentType(COMPONENT_SYSTEM);
        }

        virtual bool begin() = 0;
        virtual bool open() = 0;
        virtual void close() = 0;
        virtual void reset() = 0;
        virtual void write(IComponent &component, sd_log_id_t logId, string &customString) = 0;
};

#endif // ISDCARD_H