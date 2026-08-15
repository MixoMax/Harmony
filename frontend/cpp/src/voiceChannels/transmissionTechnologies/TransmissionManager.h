//
// Created by tim on 15.08.26.
//

#ifndef HARMONY_TRANSMISSIONMANAGER_H
#define HARMONY_TRANSMISSIONMANAGER_H


class TransmissionManager {
public:
    virtual ~TransmissionManager() = default;

    virtual void connect();

    virtual void disconnect();

    virtual void send();

    virtual void setReceiveCallback(void (*callback)(const char *data, int length));
};


#endif //HARMONY_TRANSMISSIONMANAGER_H
