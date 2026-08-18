//
// Created by tim on 15.08.26.
//

#ifndef HARMONY_WEBSOCKETMANAGER_H
#define HARMONY_WEBSOCKETMANAGER_H
#include "../TransmissionManager.h"
#include "ixwebsocket/IXWebSocket.h"


class WebsocketManager : public TransmissionManager {
    ix::WebSocket webSocket;

public:
    ~WebsocketManager() override;

    void connect() override;

    void disconnect() override;

    void send(char *data, size_t length) override;
};


#endif //HARMONY_WEBSOCKETMANAGER_H
