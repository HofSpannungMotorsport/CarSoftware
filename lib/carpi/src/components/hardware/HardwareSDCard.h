#ifndef HARDWARE_SD_CARD_H
#define HARDWARE_SD_CARD_H

#include <string>

using namespace std;

#include "SDFileSystem.h"
#include "../interface/ISDCard.h"

#define SD_CARD_TRANSFER_SPEED 16000000 // -> 16 MHz

class HardwareSDCard : public ISDCard {
    public:
        HardwareSDCard(id_sub_component_t componentSubId, PinName mosi, PinName miso, PinName sck, PinName cs)
        : sd(mosi, miso, sck, cs, SD_LOG_ROOT, SD_CARD_TRANSFER_SPEED) {
            setComponentType(COMPONENT_SYSTEM);
            setComponentSubId(componentSubId);
            setObjectType(OBJECT_HARDWARE);
        }

        virtual bool begin() {
            sd.disk_initialize();
            if (sd.disk_status() == 0) {
                mkdir(SD_LOG_FOLDER_PATH, 0777);
                return true;
            }

            _status |= SD_INITALIZATION_FAILED;
            return false;
        }

        virtual bool open() {
            // Check if file was opened before
            if (_fileOpen) return true;

            // Check if the SD Card was initialized correctly
            if (sd.disk_status() != 0) {
                _status |= SD_NOT_INITIALIZED_BEFORE;
                return false;
            }

            fp = fopen(SD_LOG_FILE_PATH, "a");
            if(fp == NULL) {
                _status |= SD_FILE_ACCESS_FAILED;
                _fileOpen = false;
                return false;
            }

            _fileOpen = true;
            return true;
        }

        virtual void close() {
            if (fp != NULL) {
                fclose(fp);
            }
            _fileOpen = false;
        }

        virtual void reset() {
            _fileOpen = false;
            fp = NULL;
            _status = SD_OK;

            sd.disk_initialize();
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
        bool _fileOpen = false;

        FILE *fp;

        void _addLineHeader(string &line, IComponent &component, sd_log_id_t logId) {
            line += to_string(us_ticker_read()/1000) + ";" + to_string(component.getComponentId()) + ";" + to_string(logId) + ";";
        }

        void _writeLine(string &line) {
            // Check if the SD Card was initialized correctly
            if (sd.disk_status() != 0) {
                _status |= SD_NOT_INITIALIZED_BEFORE;
                return;
            }

            line += "\r\n";

            if (_fileOpen) {
                fprintf(fp, "%s", line.c_str());
            } else {
                _status |= SD_FILE_NOT_OPENED_BEFORE;
            }
        }
};

#endif // HARDWARE_SD_CARD_H