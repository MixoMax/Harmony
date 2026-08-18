#include "VoiceChannelManager.h"

#include <iostream>
#include <thread>
#include <utility>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketMessage.h>
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
        bufferAttr.tlength = packageSize;
        bufferAttr.prebuf = static_cast<uint32_t>(-1);
        bufferAttr.minreq = static_cast<uint32_t>(-1);
        bufferAttr.fragsize = packageSize;


        while (!stopThread) {
            shouldConnect.wait(false);
            if (stopThread) {
                break;
            }
            mutex.lock();

            int paCaptureErrorCode;
            pa_simple *captureStream = pa_simple_new(
                nullptr, "Harmony", PA_STREAM_RECORD,
                nullptr, "AudioCapture", &spec, nullptr, &bufferAttr, &paCaptureErrorCode);
            if (!captureStream) {
                fprintf(stderr, "AudioCapture failed: %s\n", pa_strerror(paCaptureErrorCode));
                return 1;
            }
            int8_t captureBuffer[packageSize];

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
                    if (!playbackStream) {
                        return;
                    }
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
                    //TransmissionManager lost connection
                    break;
                }
                if (!TransmissionManager::instance->isConnected) {
                    //TransmissionManager is not connected yet
                    continue;
                }
                wasConnected = true;


                /*capture audio*/
                if (pa_simple_read(captureStream, captureBuffer, packageSize * sizeof(int8_t),
                                   &paCaptureErrorCode) < 0) {
                    fprintf(stderr, "pa_simple_read() failed: %s\n", pa_strerror(paCaptureErrorCode));
                    break;
                }
                TransmissionManager::instance->send(reinterpret_cast<char *>(captureBuffer),
                                                    packageSize);
            }

            TransmissionManager::instance->disconnect();


            pa_simple_free(captureStream);

            pa_simple_drain(playbackStream, &paCaptureErrorCode);
            pa_simple_free(playbackStream);
            playbackStream = nullptr;
        }
        std::cout << "> VoiceChannelManager Thread stopped" << std::endl;
        return 0;
    });
}

VoiceChannelManager &VoiceChannelManager::getInstance() {
    static VoiceChannelManager instance;
    return instance;
}

void VoiceChannelManager::connect(std::string roomName, std::string userName) {
    auto &instance = getInstance();

    instance.mutex.lock();
    instance.roomName = std::move(roomName);
    instance.userName = std::move(userName);
    instance.mutex.unlock();
    instance.shouldConnect = true;
    instance.shouldConnect.notify_all();
}

void VoiceChannelManager::disconnect() {
    std::cout << "> VoiceChannelManager disconnecting..." << std::endl;
    auto &instance = getInstance();

    instance.shouldConnect = false;
}

void VoiceChannelManager::setDeviceNames(std::string micDeviceName, std::string speakerDeviceName) {
    auto &instance = getInstance();

    instance.mutex.lock();
    instance.micDeviceName = std::move(micDeviceName);
    instance.speakerDeviceName = std::move(speakerDeviceName);
    instance.mutex.unlock();
}

void VoiceChannelManager::join() {
    auto &instance = getInstance();
    instance.stopThread = true;
    instance.shouldConnect = true;
    instance.shouldConnect.notify_all();
    instance.thread.join();

    TransmissionManager::cleanUp();
}
