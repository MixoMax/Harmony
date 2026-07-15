#ifndef HARMONY_PULSEAUDIODEVICEENUMERATION_H
#define HARMONY_PULSEAUDIODEVICEENUMERATION_H
#include <string>
#include <vector>
#include <pulse/context.h>
#include <pulse/introspect.h>
#include <pulse/mainloop.h>

struct PADevice {
    std::string name;
    std::string description;
};

struct PAEnumData {
    pa_mainloop *m{};
    std::vector<PADevice> sinks;
    std::vector<PADevice> sources;
    std::string default_sink;
    std::string default_source;
    int ops_pending{};
};

void pa_sink_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
    (void) c;
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
    (void) c;
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
    (void) c;
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


#endif //HARMONY_PULSEAUDIODEVICEENUMERATION_H
