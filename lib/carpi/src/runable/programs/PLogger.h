#ifndef LOGGER_H
#define LOGGER_H

#include <vector>
#include <memory>
#include <string>
using namespace std;

#include "IProgram.h"
#include "../services/SCar.h"
#include "components/interface/ISDCard.h"
#include "components/interface/IComponent.h"

#define P_LOGGER_VERSION "Logger_Version_V0.0.1"
#define STD_LOGGER_REFRESH_RATE 1

enum p_logger_sd_log_id_t : sd_log_id_t {
    P_LOGGER_SD_LOG_ID_BEGIN = 0x1,
    P_LOGGER_SD_LOG_ID_VERSION = 0x2
};

enum log_component_type_t {
    UINT8_T = 0x1,
    UINT16_T,
    UINT32_T,
    FLOAT,
    INT16_T
};

class PLogger : public IProgram {
    public:
        PLogger(SCar &carService, ISDCard &sdCard)
        : _carService(carService), _sdCard(sdCard) {
            string beginMessage = "Begin_Logging";
            string loggerVersion = P_LOGGER_VERSION;
            _sdCard.writeCustom(_sdCard, P_LOGGER_SD_LOG_ID_BEGIN, beginMessage);
            _sdCard.writeCustom(_sdCard, P_LOGGER_SD_LOG_ID_VERSION, loggerVersion);
        }

        virtual void run() {
            if (_carService.getState() != READY_TO_DRIVE) return;

            for (LogComponent &logComponent : _logComponents) {
                if (logComponent.running) {
                    if (logComponent.lastRun->read() > (1.0 / logComponent.refreshRate)) {
                        logComponent.lastRun->reset();
                        logComponent.send(_sdCard);
                    }
                } else {
                    logComponent.send(_sdCard);
                    logComponent.lastRun->reset();
                    logComponent.lastRun->start();
                    logComponent.running = true;
                }
            }
        }

        void addComponentToLog(IComponent &component, uint8_t (*logFunction)(), sd_log_id_t logId, float refreshRate = STD_LOGGER_REFRESH_RATE) {
            _logComponents.emplace_back(component, logFunction, logId, refreshRate);
        }

        void addComponentToLog(IComponent &component, uint16_t (*logFunction)(), sd_log_id_t logId, float refreshRate = STD_LOGGER_REFRESH_RATE) {
            _logComponents.emplace_back(component, logFunction, logId, refreshRate);
        }

        void addComponentToLog(IComponent &component, uint32_t (*logFunction)(), sd_log_id_t logId, float refreshRate = STD_LOGGER_REFRESH_RATE) {
            _logComponents.emplace_back(component, logFunction, logId, refreshRate);
        }

        void addComponentToLog(IComponent &component, float (*logFunction)(), sd_log_id_t logId, float refreshRate = STD_LOGGER_REFRESH_RATE) {
            _logComponents.emplace_back(component, logFunction, logId, refreshRate);
        }

        void addComponentToLog(IComponent &component, int16_t (*logFunction)(), sd_log_id_t logId, float refreshRate = STD_LOGGER_REFRESH_RATE) {
            _logComponents.emplace_back(component, logFunction, logId, refreshRate);
        }

        void finalize() {
            _logComponents.shrink_to_fit();
        }
    
    protected:
        class LogComponent {
            public:
                LogComponent(IComponent &_component, uint8_t (*logFunction)(), sd_log_id_t _logId, float _refreshRate) {
                    logFunction8 = logFunction;
                    _initRest(_component, UINT8_T, _logId, _refreshRate);
                }

                LogComponent(IComponent &_component, uint16_t (*logFunction)(), sd_log_id_t _logId, float _refreshRate) {
                    logFunction16 = logFunction;
                    _initRest(_component, UINT16_T, _logId, _refreshRate);
                }

                LogComponent(IComponent &_component, uint32_t (*logFunction)(), sd_log_id_t _logId, float _refreshRate) {
                    logFunction32 = logFunction;
                    _initRest(_component, UINT32_T, _logId, _refreshRate);
                }

                LogComponent(IComponent &_component, float (*logFunction)(), sd_log_id_t _logId, float _refreshRate) {
                    logFunctionF = logFunction;
                    _initRest(_component, FLOAT, _logId, _refreshRate);
                }
                LogComponent(IComponent &_component, int16_t (*logFunction)(), sd_log_id_t _logId, float _refreshRate) {
                    logFunctionS16 = logFunction;
                    _initRest(_component, INT16_T, _logId, _refreshRate);
                }

                void send(ISDCard &sdCard) {
                    switch(type) {
                        case UINT8_T:
                            sdCard.writeValue(*component, logId, logFunction8());
                            break;
                        
                        case UINT16_T:
                            sdCard.writeValue(*component, logId, logFunction16());
                            break;
                        
                        case UINT32_T:
                            sdCard.writeValue(*component, logId, logFunction32());
                            break;
                        
                        case FLOAT:
                            sdCard.writeValue(*component, logId, logFunctionF());
                            break;

                        case INT16_T:
                            sdCard.writeValue(*component, logId, logFunctionS16());
                            break;
                    }
                }

                IComponent *component;

                log_component_type_t type;
                uint8_t (*logFunction8)();
                uint16_t (*logFunction16)();
                uint32_t (*logFunction32)();
                float (*logFunctionF)();
                int16_t (*logFunctionS16)();

                sd_log_id_t logId;

                float refreshRate;
                bool running = false;

                shared_ptr<Timer> lastRun;
    
            protected:
                void _initRest(IComponent &_component, log_component_type_t _type, sd_log_id_t _logId, float _refreshRate) {
                    lastRun = shared_ptr<Timer>(new Timer());
                    component = &_component;
                    type = _type;
                    logId = _logId;
                    refreshRate = _refreshRate;
                }
            
            private:
                LogComponent() {}
        };

        vector<LogComponent> _logComponents;

        ISDCard &_sdCard;
        SCar &_carService;
};

#endif // LOGGER_H