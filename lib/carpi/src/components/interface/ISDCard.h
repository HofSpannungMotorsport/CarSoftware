#ifndef ISDCARD_H
#define ISDCARD_H

#include <string>
using namespace std;

#include "IComponent.h"

#define SD_LOG_ROOT "sd"
#define SD_LOG_FOLDER_PATH "/sd/log"
#define SD_LOG_FILE_PATH "/sd/log/log.txt"

#define SD_CARD_SD_LOG_COUNT 2
enum sd_log_id_sd_card_t : sd_log_id_t {
    SD_LOG_ID_BEGIN_FILE = 0,
    SD_LOG_ID_CARPI_VERSION = 1
};

typedef uint8_t sd_status_t;
enum sd_error_type_t : sd_status_t {
    SD_OK = 0x0,
    SD_INITALIZATION_FAILED = 0x1
};

class ISDCard : public IComponent {
    public:
        virtual void setStatus(sd_status_t status) = 0;
        virtual sd_status_t getStatus() = 0;

        virtual void write(IComponent &component, sd_log_id_t logId, string &customString) = 0;
};

#endif // ISDCARD_H