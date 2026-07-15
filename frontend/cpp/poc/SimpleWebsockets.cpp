#include <atomic>
#include <fstream>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <ixwebsocket/IXWebSocket.h>
#include <iostream>
#include <thread>
#include <vector>

std::atomic_bool running{true};
std::atomic_bool hasConnection{false};
constexpr int packageSize = 1024;

int main(const int argc, const char *argv[]) {
    std::cout << "> Starting Client..." << std::endl;

    std::cout << "> Parsing Program Arguments..." << std::endl;
    std::string roomName = "test";
    std::string userName = "tim";
    const std::string serverURL = "harmony.linush.org";

    int currentArgumentIndex = 0;
    while (currentArgumentIndex < argc) {
        const std::string argument(argv[currentArgumentIndex]);

        if (argument == "--room") {
            roomName = argv[currentArgumentIndex + 1];
            currentArgumentIndex += 2;
        } else if (argument == "--name") {
            userName = argv[currentArgumentIndex + 1];
            currentArgumentIndex += 2;
        } else {
            currentArgumentIndex++;
        }
    }

    std::cout << "> Initialize Audio Streams..." << std::endl;
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

    ix::WebSocket webSocket;
    webSocket.setUrl("wss://" + serverURL + "/ws/" + roomName + "/" + userName);
    webSocket.setOnMessageCallback(
        [&playbackStream, &paPlaybackErrorCode](const ix::WebSocketMessagePtr &msg) {
            switch (msg->type) {
                case ix::WebSocketMessageType::Open:
                    std::cout << "> Connected to Server" << std::endl;
                    hasConnection.store(true);
                    break;
                case ix::WebSocketMessageType::Close:
                    std::cout << "> Disconnected from Server" << std::endl;
                    hasConnection.store(false);
                    running.store(false);
                    break;
                case ix::WebSocketMessageType::Error:
                    std::cout << "> !WS ERROR!: " << msg->errorInfo.reason << std::endl;
                    running.store(false);
                    break;
                case ix::WebSocketMessageType::Message:
                    /*receiving audio*/
                    if (pa_simple_write(playbackStream, msg->str.data(), msg->str.size(),
                                        &paPlaybackErrorCode) < 0) {
                        fprintf(stderr, "pa_simple_write() failed: %s\n", pa_strerror(paPlaybackErrorCode));
                        break;
                    }
                    break;
                default:
                    std::cout << "> Unknown message type: " << static_cast<int>(msg->type) << std::endl;
                    break;
            }
        });
    webSocket.enableAutomaticReconnection();
    webSocket.start();

    auto inputThread = std::thread{
        [] {
            std::string input;
            std::cin >> input;

            running.store(false);
        }
    };

    while (running.load()) {
        if (!hasConnection.load()) {
            continue;
        }

        /*capture audio*/
        if (pa_simple_read(captureStream, captureBuffer.data(), captureBuffer.size() * sizeof(int16_t),
                           &paCaptureErrorCode) < 0) {
            fprintf(stderr, "pa_simple_read() failed: %s\n", pa_strerror(paCaptureErrorCode));
            break;
        }
        std::string payload{
            reinterpret_cast<const char *>(captureBuffer.data()),
            captureBuffer.size() * sizeof(int16_t)
        };
        if (!webSocket.sendBinary(payload).success) {
            std::cerr << "failed to send binaries" << std::endl;
        }
    }

    webSocket.close();

    pa_simple_free(captureStream);

    pa_simple_drain(playbackStream, &paCaptureErrorCode);
    pa_simple_free(playbackStream);

    inputThread.join();

    std::cout << "> stopped successfully" << std::endl;

    return 0;
}
