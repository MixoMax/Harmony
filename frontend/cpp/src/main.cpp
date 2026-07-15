#include <atomic>
#include <fstream>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/pulseaudio.h>
#include <ixwebsocket/IXWebSocket.h>
#include <iostream>
#include <thread>
#include <vector>
#include <string>

std::atomic_bool running{true};
std::atomic_bool hasConnection{false};
constexpr int packageSize = 1024;

// --- PulseAudio Device Enumeration ---
struct PADevice {
    std::string name;
    std::string description;
};

struct PAEnumData {
    pa_mainloop *m;
    std::vector<PADevice> sinks;
    std::vector<PADevice> sources;
    std::string default_sink;
    std::string default_source;
    int ops_pending;
};

void pa_sink_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
    (void)c;
    PAEnumData *data = static_cast<PAEnumData *>(userdata);
    if (eol > 0) {
        data->ops_pending--;
        if (data->ops_pending == 0) pa_mainloop_quit(data->m, 0);
        return;
    }
    if (i && i->name && i->description) {
        data->sinks.push_back({i->name, i->description});
    }
}

void pa_source_cb(pa_context *c, const pa_source_info *i, int eol, void *userdata) {
    (void)c;
    PAEnumData *data = static_cast<PAEnumData *>(userdata);
    if (eol > 0) {
        data->ops_pending--;
        if (data->ops_pending == 0) pa_mainloop_quit(data->m, 0);
        return;
    }
    if (i && i->name && i->description) {
        data->sources.push_back({i->name, i->description});
    }
}

void pa_server_cb(pa_context *c, const pa_server_info *i, void *userdata) {
    (void)c;
    PAEnumData *data = static_cast<PAEnumData *>(userdata);
    if (i) {
        if (i->default_sink_name) data->default_sink = i->default_sink_name;
        if (i->default_source_name) data->default_source = i->default_source_name;
    }
    data->ops_pending--;
    if (data->ops_pending == 0) pa_mainloop_quit(data->m, 0);
}

void pa_state_cb(pa_context *c, void *userdata) {
    PAEnumData *data = static_cast<PAEnumData *>(userdata);
    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_READY: {
            data->ops_pending = 3;
            pa_context_get_sink_info_list(c, pa_sink_cb, data);
            pa_context_get_source_info_list(c, pa_source_cb, data);
            pa_context_get_server_info(c, pa_server_cb, data);
            break;
        }
        case PA_CONTEXT_FAILED:
        case PA_CONTEXT_TERMINATED:
            pa_mainloop_quit(data->m, 0);
            break;
        default:
            break;
    }
}

PAEnumData enumerate_audio_devices() {
    PAEnumData data;
    data.ops_pending = 0;
    data.m = pa_mainloop_new();
    if (!data.m) return data;

    pa_mainloop_api *api = pa_mainloop_get_api(data.m);
    pa_context *ctx = pa_context_new(api, "Harmony Enum");
    if (!ctx) {
        pa_mainloop_free(data.m);
        return data;
    }

    pa_context_set_state_callback(ctx, pa_state_cb, &data);
    if (pa_context_connect(ctx, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0) {
        pa_context_unref(ctx);
        pa_mainloop_free(data.m);
        return data;
    }

    int ret = 0;
    pa_mainloop_run(data.m, &ret);

    pa_context_disconnect(ctx);
    pa_context_unref(ctx);
    pa_mainloop_free(data.m);

    return data;
}


int main(const int argc, const char *argv[]) {
    std::cout << "> Starting Client..." << std::endl;

    std::cout << "> Parsing Program Arguments..." << std::endl;
    std::string roomName = "test";
    std::string userName = "tim";
    std::string micDeviceName = "";
    std::string speakerDeviceName = "";
    const std::string serverURL = "harmony.linush.org";

    int currentArgumentIndex = 0;
    while (currentArgumentIndex < argc) {
        const std::string argument(argv[currentArgumentIndex]);

        if (argument == "--room" && currentArgumentIndex + 1 < argc) {
            roomName = argv[currentArgumentIndex + 1];
            currentArgumentIndex += 2;
        } else if (argument == "--name" && currentArgumentIndex + 1 < argc) {
            userName = argv[currentArgumentIndex + 1];
            currentArgumentIndex += 2;
        } else if (argument == "--mic" && currentArgumentIndex + 1 < argc) {
            micDeviceName = argv[currentArgumentIndex + 1];
            currentArgumentIndex += 2;
        } else if (argument == "--speaker" && currentArgumentIndex + 1 < argc) {
            speakerDeviceName = argv[currentArgumentIndex + 1];
            currentArgumentIndex += 2;
        } else {
            currentArgumentIndex++;
        }
    }

    std::cout << "> Enumerating Audio Devices..." << std::endl;
    PAEnumData paData = enumerate_audio_devices();

    std::cout << "\n--- Available Microphones (Inputs) ---" << std::endl;
    for (const auto& src : paData.sources) {
        bool isDefault = (src.name == paData.default_source);
        std::cout << (isDefault ? "[*] " : "[-] ") << src.name << "\n    " << src.description << std::endl;
    }

    std::cout << "\n--- Available Speakers (Outputs) ---" << std::endl;
    for (const auto& sink : paData.sinks) {
        bool isDefault = (sink.name == paData.default_sink);
        std::cout << (isDefault ? "[*] " : "[-] ") << sink.name << "\n    " << sink.description << std::endl;
    }

    std::string activeMic = micDeviceName.empty() ? paData.default_source : micDeviceName;
    std::string activeSpeaker = speakerDeviceName.empty() ? paData.default_sink : speakerDeviceName;

    std::cout << "\n> Using Microphone: " << activeMic << std::endl;
    std::cout << "> Using Speaker: " << activeSpeaker << "\n" << std::endl;

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

    const char* paMic = micDeviceName.empty() ? nullptr : micDeviceName.c_str();
    int paCaptureErrorCode;
    pa_simple *captureStream = pa_simple_new(
        nullptr, "Harmony", PA_STREAM_RECORD,
        paMic, "AudioCapture", &spec, nullptr, &bufferAttr, &paCaptureErrorCode);
    if (!captureStream) {
        fprintf(stderr, "AudioCapture failed: %s\n", pa_strerror(paCaptureErrorCode));
        return 1;
    }
    std::vector<int16_t> captureBuffer(packageSize);

    const char* paSpeaker = speakerDeviceName.empty() ? nullptr : speakerDeviceName.c_str();
    int paPlaybackErrorCode;
    pa_simple *playbackStream = pa_simple_new(
        nullptr, "Harmony", PA_STREAM_PLAYBACK,
        paSpeaker, "AudioPlayback", &spec, nullptr, &bufferAttr, &paPlaybackErrorCode);
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