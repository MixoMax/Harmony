/**
 * Proof of Concept file
 */

#include <atomic>
#include <fstream>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <ixwebsocket/IXWebSocket.h>
#include <iostream>
#include <thread>
#include <vector>

std::atomic_bool running{true};
constexpr int packageSize = 1024;

int main() {
    std::cout << "> Starting Client..." << std::endl;

    std::cout << "> Initialize Audio Streams..." << std::endl;
    pa_sample_spec spec;
    spec.format = PA_SAMPLE_S16LE;
    spec.rate = 44100;
    spec.channels = 1;

    int paCaptureErrorCode;
    pa_simple *captureStream = pa_simple_new(
        nullptr, "Harmony", PA_STREAM_RECORD,
        nullptr, "AudioCapture", &spec, nullptr, nullptr, &paCaptureErrorCode);
    if (!captureStream) {
        fprintf(stderr, "AudioCapture failed: %s\n", pa_strerror(paCaptureErrorCode));
        return 1;
    }
    std::vector<int16_t> captureBuffer(packageSize);

    int paPlaybackErrorCode;
    pa_simple *playbackStream = pa_simple_new(
        nullptr, "Harmony", PA_STREAM_PLAYBACK,
        nullptr, "AudioPlayback", &spec, nullptr, nullptr, &paPlaybackErrorCode);
    if (!playbackStream) {
        fprintf(stderr, "AudioPlayback failed: %s\n", pa_strerror(paPlaybackErrorCode));
        return 1;
    }

    std::cout << "> Starting Capturing..." << std::endl;


    auto inputThread = std::thread{
        [] {
            std::string input;
            std::cin >> input;

            running.store(false);
        }
    };

    /*capture audio*/
    while (running.load()) {
        if (pa_simple_read(captureStream, captureBuffer.data(), captureBuffer.size() * sizeof(int16_t),
                           &paCaptureErrorCode) < 0) {
            fprintf(stderr, "pa_simple_read() failed: %s\n", pa_strerror(paCaptureErrorCode));
            break;
        }

        int16_t max = 0;
        for (const auto sample: captureBuffer) {
            max = std::max(max, static_cast<int16_t>(std::abs(sample)));
        }
        std::cout << "Amplitude: "<< max << std::endl;

        if (pa_simple_write(playbackStream, captureBuffer.data(), captureBuffer.size() * sizeof(int16_t),
                            &paPlaybackErrorCode) < 0) {
            fprintf(stderr, "pa_simple_write() failed: %s\n", pa_strerror(paPlaybackErrorCode));
        }
    }


    pa_simple_free(captureStream);

    pa_simple_drain(playbackStream, &paCaptureErrorCode);
    pa_simple_free(playbackStream);

    inputThread.join();

    std::cout << "> stopped successfully" << std::endl;

    return 0;
}
