#ifndef HARDWARE_SD_CARD_H
#define HARDWARE_SD_CARD_H

#include <string>

using namespace std;

#include "SDFileSystem.h"
#include "../interface/ISDCard.h"

class HardwareSDCard : public ISDCard {
    public:
        HardwareSDCard(PinName mosi, PinName miso, PinName sck, PinName cs)
        : sd(mosi, miso, sck, cs, SD_LOG_ROOT) {
            mkdir(SD_LOG_FOLDER_PATH, 0777);
        }

        virtual void setStatus(status_t status) {
            // No implementation needed
        }

        virtual status_t getStatus() {
            return _status;
        }

        virtual void write(IComponent &component, sd_log_id_t logId, string &customString) {
            string line = "";
            _addLineHeader(line, component, logId);
            line += customString + ";";
            _writeLine(line);
        }

    protected:
        SDFileSystem sd;

        status_t _status = SD_OK;

        void _addLineHeader(string &line, IComponent &component, sd_log_id_t logId) {
            line += to_string(component.getComponentId()) + ";" + to_string(logId) + ";";
        }

        void _writeLine(string &line) {
            // Check if the SD Card was initialized correctly
            if (sd.disk_status() != 0) {
                _status |= SD_INITALIZATION_FAILED;
                return;
            }

            line += "\r\n";

            FILE *fp = fopen(SD_LOG_FILE_PATH, "a");
            if(fp == NULL) {
                _status |= SD_FILE_ACCESS_FAILED;
            } else {
                fprintf(fp, "%s", line.c_str());
            }

            fclose(fp);
        }
};

#endif // HARDWARE_SD_CARD_H