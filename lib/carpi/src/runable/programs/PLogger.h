#ifndef P_LOGGER_H
#define P_LOGGER_H

#include <string>
#include <vector>
using namespace std;

#include "IProgram.h"
#include "../services/SCar.h"
#include "components/interface/IComponent.h"
#include "components/interface/ISDCard.h"

#define STD_LOGGER_REFRESH_RATE 1
#define STD_FILE_BEGIN_MESSAGE "Begin_Logging"

class PLogger : public IProgram {
    public:
        PLogger(SCar &carService, ISDCard &sdCard)
        : _sdCard(sdCard), _carService(carService) {}

        bool begin() {
            if (_sdCard.getStatus() != 0) return false;

            string beginMessage = STD_FILE_BEGIN_MESSAGE;
            _sdCard.write(_sdCard, SD_LOG_ID_BEGIN_FILE, beginMessage);

            string versionMessage = CARPI_VERSION;
            _sdCard.write(_sdCard, SD_LOG_ID_CARPI_VERSION, versionMessage);

            begun = true;
            return true;
        }

        virtual void run() {
            // Only capture values if in READY_TO_DRIVE mode
            if (_carService.getState() != READY_TO_DRIVE) return;

            // Check for errors
            if (!begun) return;
            if (_sdCard.getStatus() != 0) return;

            // At first the Values
            for (LoggingValue &loggingValue : _loggingValues) {
                if (loggingValue.logged) {
                    if (loggingValue.lastRun->read() > (1.0 / loggingValue.refreshRate)) {
                        loggingValue.lastRun->reset();
                        loggingValue.send(_sdCard);
                    }
                } else {
                    loggingValue.send(_sdCard);
                    loggingValue.lastRun->reset();
                    loggingValue.lastRun->start();
                    loggingValue.logged = true;
                }
            }

            // Then the complete Components
            for (LoggingComponent &loggingComponent : _loggingComponents) {
                if (loggingComponent.logged) {
                    if (loggingComponent.lastRun->read() > (1.0 / loggingComponent.refreshRate)) {
                        loggingComponent.lastRun->reset();
                        loggingComponent.send(_sdCard);
                    }
                } else {
                    loggingComponent.send(_sdCard);
                    loggingComponent.lastRun->reset();
                    loggingComponent.lastRun->start();
                    loggingComponent.logged = true;
                }
            }
        }

        void addLogableValue(IComponent &logable, sd_log_id_t logId, float refreshRate = STD_LOGGER_REFRESH_RATE) {
            _loggingValues.emplace_back(logable, logId, refreshRate);
        }

        void addLogableComponent(IComponent &logable, float refreshRate = STD_LOGGER_REFRESH_RATE) {
            _loggingComponents.emplace_back(logable, refreshRate);
        }

        void finalize() {
            _loggingValues.shrink_to_fit();
            _loggingComponents.shrink_to_fit();
        }
    
    protected:
        ISDCard &_sdCard;
        SCar &_carService;

        bool begun = false;

        class LoggingComponent {
            public:
                LoggingComponent(IComponent &_logable, float _refreshRate)
                : logable(&_logable), refreshRate(_refreshRate) {
                    lastRun = std::shared_ptr<Timer>(new Timer());
                }

                virtual void send(ISDCard &sdCard) {
                    for (sd_log_id_t i = 0; i < logable->getLogValueCount(); i++) {
                        string logValue = "";
                        logable->getLogValue(logValue, i);
                        sdCard.write(*logable, i, logValue);
                    }
                }

                IComponent *logable;

                float refreshRate = STD_SCHEDULER_REFRESH_RATE;
                bool logged = false;

                std::shared_ptr<Timer> lastRun;
        };

        class LoggingValue : public LoggingComponent {
            public:
                LoggingValue(IComponent &_logable, sd_log_id_t _logId, float _refreshRate)
                : LoggingComponent(_logable, _refreshRate), logId(_logId) {}

                virtual void send(ISDCard &sdCard) {
                    string logValue = "";
                    logable->getLogValue(logValue, logId);
                    sdCard.write(*logable, logId, logValue);
                }

                sd_log_id_t logId;
        };

        vector<LoggingValue> _loggingValues;
        vector<LoggingComponent> _loggingComponents;
};

#endif // P_LOGGER_H