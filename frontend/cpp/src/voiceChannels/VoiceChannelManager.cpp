#include "VoiceChannelManager.h"

#include <iostream>
#include <thread>
#include <utility>
#include <ixwebsocket/IXProgressCallback.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketMessage.h>
#include <ixwebsocket/IXWebSocketMessageType.h>
#include <pulse/def.h>
#include <pulse/error.h>
#include <pulse/sample.h>
#include <pulse/simple.h>

#include "../settings.h"
#include "transmissionTechnologies/TransmissionManager.h"


VoiceChannelManager::VoiceChannelManager() {
    thread = std::thread([this] {
        std::cout << "> Starting Voice Channel Manager" << std::endl;
        pa_sample_spec spec;
        spec.format = PA_SAMPLE_S16LE;
        spec.rate = 16000; //44100 for music, 16000/22050 for us
        spec.channels = 1;

        pa_buffer_attr bufferAttr;
        bufferAttr.maxlength = static_cast<uint32_t>(-1);
        bufferAttr.tlength = packageSize * sizeof(int16_t) * 2;
        bufferAttr.prebuf = static_cast<uint32_t>(-1);
        bufferAttr.minreq = static_cast<uint32_t>(-1);
        bufferAttr.fragsize = sizeof(int16_t);


        while (!stopThread) {
            shouldConnect.wait(false);
            if (stopThread) {
                break;
            }
            std::cout << "> Starting Websocket" << std::endl;
            mutex.lock();

            int paCaptureErrorCode;
            pa_simple *captureStream = pa_simple_new(
                nullptr, "Harmony", PA_STREAM_RECORD,
                nullptr, "AudioCapture", &spec, nullptr, &bufferAttr, &paCaptureErrorCode);
            if (!captureStream) {
                fprintf(stderr, "AudioCapture failed: %s\n", pa_strerror(paCaptureErrorCode));
                return 1;
            }
            std::vector<int16_t> captureBuffer(packageSize);

            int paPlaybackErrorCode;
            pa_simple *playbackStream = pa_simple_new(
                nullptr, "Harmony", PA_STREAM_PLAYBACK,
                nullptr, "AudioPlayback", &spec, nullptr, &bufferAttr, &paPlaybackErrorCode);
            if (!playbackStream) {
                fprintf(stderr, "AudioPlayback failed: %s\n", pa_strerror(paPlaybackErrorCode));
                return 1;
            }

            std::cout << "> Opening Server-connection..." << std::endl;

            TransmissionManager::instance->roomName = roomName;
            TransmissionManager::instance->userName = userName;
            TransmissionManager::instance->setReceiveCallback(
                [&playbackStream, &paPlaybackErrorCode](const char *data, const int length) {
                    if (pa_simple_write(playbackStream, data, length,
                                        &paPlaybackErrorCode) < 0) {
                        fprintf(stderr, "pa_simple_write() failed: %s\n", pa_strerror(paPlaybackErrorCode));
                    }
                });
            TransmissionManager::instance->connect();
            mutex.unlock();

            bool wasConnected{false};

            while (shouldConnect && !stopThread) {
                if (!TransmissionManager::instance->isConnected && wasConnected) {
                    //ws disconnected
                    break;
                }
                if (!TransmissionManager::instance->isConnected) {
                    //ws connection not ready yet
                    continue;
                }
                wasConnected = true;


                /*capture audio*/
                if (pa_simple_read(captureStream, captureBuffer.data(), captureBuffer.size() * sizeof(int16_t),
                                   &paCaptureErrorCode) < 0) {
                    fprintf(stderr, "pa_simple_read() failed: %s\n", pa_strerror(paCaptureErrorCode));
                    break;
                }
                const std::string payload{
                    reinterpret_cast<const char *>(captureBuffer.data()),
                    captureBuffer.size() * sizeof(int16_t)
                };
                TransmissionManager::instance->send(payload);
            }

            TransmissionManager::instance->disconnect();


            pa_simple_free(captureStream);

            pa_simple_drain(playbackStream, &paCaptureErrorCode);
            pa_simple_free(playbackStream);
            std::cout << "> Closed Websocket" << std::endl;
        }
        std::cout << "> VoiceChannelManager Thread stopped" << std::endl;
        return 0;
    });
}

VoiceChannelManager &VoiceChannelManager::getInstance() {
    static VoiceChannelManager instance;
    return instance;
}

void VoiceChannelManager::connect(std::string roomName, std::string userName, std::string micDeviceName,
                                  std::string speakerDeviceName) {
    auto &instance = getInstance();

    instance.mutex.lock();
    instance.roomName = std::move(roomName);
    instance.userName = std::move(userName);
    instance.micDeviceName = std::move(micDeviceName);
    instance.speakerDeviceName = std::move(speakerDeviceName);
    instance.mutex.unlock();
    instance.shouldConnect = true;
    instance.shouldConnect.notify_all();
}

void VoiceChannelManager::disconnect() {
    auto &instance = getInstance();

    instance.shouldConnect = false;
}

void VoiceChannelManager::join() {
    auto &instance = getInstance();
    instance.stopThread = true;
    instance.shouldConnect = true;
    instance.shouldConnect.notify_all();
    instance.thread.join();

    TransmissionManager::cleanUp();
}
