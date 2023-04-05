#pragma once

#include "../../../HardConfig.h"
#include "../../../components/interface/ICommunication.h"
#include "../../../runable/IRunable.h"
#include "../../../steroido/src/Common/AdvancedCircularBuffer.h"
#include "../../CarMessage.h"
#include "../../componentIds.h"
#include "../../deviceIds.h"
#include "sTCPMessageTypes.h"

enum stcp_mode_t : bool { STCP_MODE_MASTER = true, STCP_MODE_SLAVE = false };

template <size_t BufferSize = STD_STCP_BUFFER_SIZE>
class sTCP : public IRunable {
   public:
    sTCP(MessageRouter& messageRouter, ChannelRouter& channelRouter, id_device_t deviceId, id_device_t myDeviceId,
         stcp_mode_t mode)
        : _messageRouter(messageRouter),
          _channelRouter(channelRouter),
          _deviceId(deviceId),
          _myDeviceId(myDeviceId),
          _mode(mode) {
        setComponentSubId(COMPONENT_SYSTEM_STCP);
        _flowControlInit = true;
        _timeoutTicker.attach(STD_STCP_INIT_TIMEOUT);
    }

    bool sendTracked(CarMessage& carMessage) {
        if (_outBuffer.full()) {
            return false;
        }

        _outBuffer.pushBack(carMessage);

        _initSend();
    }

    void receivedTracked(CarMessage& carMessage) {
        // Check if for me (lol)
    }

    void addResetCallback(ICommunication& component) {
        // Check if already in
        for (auto it = _resetCallbacks.begin(); it != _resetCallbacks.end(); ++it) {
            if (it->getComponentId() == component.getComponentId()) {
                return;
            }
        }

        // -> else, add into the list
        _resetCallbacks.push_back(&component);
    }

    void removeResetCallback(ICommunication& component) {
        // Search and remove from vector
        for (auto it = _resetCallbacks.begin(); it != _resetCallbacks.end();) {
            if (it->getComponentId() == component.getComponentId()) {
                it = _resetCallbacks.erase(it);
            } else {
                ++it;
            }
        }
    }

   private:
    MessageRouter& _messageRouter;
    ChannelRouter& _channelRouter;

    const id_device_t _myDeviceId;
    const id_device_t _deviceId;

    // Buffer
    AdvancedCircularBuffer<CarMessage, BufferSize> _outBuffer;

    // Flow Control
    stcp_mode_t _mode;
    uint8_t _nextExpectedMsgNr = 1;
    uint8_t _lastSendNr = 1;
    bool _nrSet = false;
    bool _waitForOk = false;
    bool _flowControlInit = true;

    // Status
    bool _connected = false;

    // Reset Callbacks
    vector<ICommunication*> _resetCallbacks;

    // Timeout tracking
    Ticker _timeoutTicker;

    void _incrementCounter(uint8_t& counter) {
        if (++counter == 0) counter = 1;
    }

    void _addToCounter(uint8_t& counter, uint8_t amount) {
        while (amount--) {
            _incrementCounter(counter);
        }
    }

    void _decrementCounter(uint8_t& counter) {
        if (--counter == 0) counter = 255;
    }

    void _removeFromCounter(uint8_t& counter)

    void _initSend() { _timeoutTicker.attach(callback(&_timeoutFunc), STD_STCP_MESSAGE_TIMEOUT); }

    void _timeoutFunc() {
        if (_flowControlInit) {
            // Build init message
            CarMessage message;

            message.setSenderId(_myDeviceId);
            message.setReceiverId(_deviceId);
            message.setLength(1);

            if (_mode == STCP_MODE_MASTER) {
                message.setMessageId(_nextExpectedMsgNr);
                message.set(STCP_MESSAGE_TYPE_RESET, 0);
            } else {

            }

            // Send init message
        } else {
            if (_waitForOk) {
                // Resend message
            } else {
                _timeoutTicker.detach();
            }
        }
    }
};
