#ifndef ISDCARD_H
#define ISDCARD_H

#include <string>
using namespace std;

#include "IComponent.h"

#define SD_LOG_ROOT "sd"
#define SD_LOG_FOLDER_PATH "/sd/log"
#define SD_LOG_FILE_PATH "/sd/log/log.txt"

typedef uint8_t sd_status_t;
enum sd_error_type_t : sd_status_t {
    SD_OK = 0x0,
    SD_INITALIZATION_FAILED = 0x1
};

typedef uint8_t sd_log_id_t;

class ISDCard : public IComponent {
    public:
        virtual void setStatus(sd_status_t status) = 0;
        virtual sd_status_t getStatus() = 0;

        virtual void writeValue(IComponent &component, sd_log_id_t logId, uint8_t value) = 0;
        virtual void writeValue(IComponent &component, sd_log_id_t logId, uint16_t value) = 0;
        virtual void writeValue(IComponent &component, sd_log_id_t logId, uint32_t value) = 0;
        virtual void writeValue(IComponent &component, sd_log_id_t logId, float value) = 0;
        virtual void writeValue(IComponent &component, sd_log_id_t logId, int16_t value) = 0;

        virtual void writeCustom(IComponent &component, sd_log_id_t logId, string &customString) = 0;
};

#endif // ISDCARD_H