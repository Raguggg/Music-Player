#include <gst/gst.h>
#include <stdio.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Function declarations
static void on_pad_added(GstElement *element, GstPad *pad, GstElement *sink);
static gboolean on_timeout(gpointer user_data);

typedef struct {
    GstElement *pipeline;
    GstElement *source;
    GstElement *decoder;
    GstElement *converter;
    GstElement *resampler;
    GstElement *volume;
    GstElement *sink;
    GMainLoop *loop;
} MusicPlayer;

void load_music(MusicPlayer *player, const char *sound_path);
float get_media_duration(const MusicPlayer *player);
void play_music(MusicPlayer *player);
void pause_music(MusicPlayer *player);
void resume_music(MusicPlayer *player);
void stop_music(MusicPlayer *player);
void set_volume(MusicPlayer *player, int volume);
int get_volume(const MusicPlayer *player);
int get_status(const MusicPlayer *player);
float get_position(const MusicPlayer *player);
void set_position(MusicPlayer *player, int position);
float get_duration(const MusicPlayer *player);

#ifdef __cplusplus
}
#endif

int main(int argc, char *argv[]) {
    GstElement *pipeline, *source, *decoder, *converter, *resampler, *volume, *sink;
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;
    GMainLoop *loop;
    guint timer_id;

    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create elements
    pipeline = gst_pipeline_new("audio-player");
    source = gst_element_factory_make("filesrc", "source");
    decoder = gst_element_factory_make("decodebin", "decoder");
    converter = gst_element_factory_make("audioconvert", "converter");
    resampler = gst_element_factory_make("audioresample", "resampler");
    volume = gst_element_factory_make("volume", "volume");
    sink = gst_element_factory_make("autoaudiosink", "sink");

    if (!pipeline || !source || !decoder || !converter || !resampler || !volume || !sink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    // Set properties
    g_object_set(source, "location", "tes.webm", NULL);
    g_object_set(volume, "volume", 0.1, NULL); // Set initial volume to 10%

    // Build the pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, decoder, converter, resampler, volume, sink, NULL);
    if (!gst_element_link(source, decoder)) {
        g_printerr("Source and decoder could not be linked.\n");
        gst_object_unref(pipeline);
        return -1;
    }
    if (!gst_element_link_many(converter, resampler, volume, sink, NULL)) {
        g_printerr("Converter, resampler, volume, and sink could not be linked.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // Connect the decoder to the rest of the pipeline dynamically
    g_signal_connect(decoder, "pad-added", G_CALLBACK(on_pad_added), converter);

    // Start playing
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Failed to start pipeline.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // Create and start the main loop
    loop = g_main_loop_new(NULL, FALSE);

    // Set a timeout to periodically adjust the volume
    timer_id = g_timeout_add_seconds(0.1, on_timeout, volume);

    // Run the main loop
    g_main_loop_run(loop);

    // Clean up
    g_source_remove(timer_id);
    g_main_loop_unref(loop);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}

static void on_pad_added(GstElement *element, GstPad *pad, GstElement *sink) {
    GstPad *sink_pad = gst_element_get_static_pad(sink, "sink");
    if (gst_pad_is_linked(sink_pad)) {
        g_object_unref(sink_pad);
        return;
    }

    GstPadLinkReturn ret = gst_pad_link(pad, sink_pad);
    if (GST_PAD_LINK_FAILED(ret)) {
        g_printerr("Type is '%s' but link failed.\n", GST_PAD_NAME(pad));
    } else {
        g_print("Link succeeded (type '%s').\n", GST_PAD_NAME(pad));
    }

    gst_object_unref(sink_pad);
}

// Timeout callback to adjust volume
static gboolean on_timeout(gpointer user_data) {
    static gdouble volume = 0.1;
    GstElement *volume_element = GST_ELEMENT(user_data);

    // volume += 0.1;
    // if (volume > 1.0) {
    //     volume = 0.1;
    // }

    g_object_set(volume_element, "volume", volume, NULL);
    g_print("Volume set to %.1f\n", volume);

    return TRUE; // Continue calling this function
}

void load_music(MusicPlayer *player, const char *sound_path) {
    g_object_set(player->source, "location", sound_path, NULL);
}

float get_media_duration(const MusicPlayer *player) {
    gint64 duration = 0;
    if (gst_element_query_duration(player->pipeline, GST_FORMAT_TIME, &duration)) {
        return (float)duration / GST_SECOND;
    }
    return -1.0;
}

void play_music(MusicPlayer *player) {
    gst_element_set_state(player->pipeline, GST_STATE_PLAYING);
}

void pause_music(MusicPlayer *player) {
    gst_element_set_state(player->pipeline, GST_STATE_PAUSED);
}

void resume_music(MusicPlayer *player) {
    gst_element_set_state(player->pipeline, GST_STATE_PLAYING);
}

void stop_music(MusicPlayer *player) {
    gst_element_set_state(player->pipeline, GST_STATE_NULL);
}

void set_volume(MusicPlayer *player, int volume) {
    gdouble vol = (gdouble)volume / 100.0;
    g_object_set(player->volume, "volume", vol, NULL);
}

int get_volume(const MusicPlayer *player) {
    gdouble volume = 0.0;
    g_object_get(player->volume, "volume", &volume, NULL);
    return (int)(volume * 100);
}

int get_status(const MusicPlayer *player) {
    GstState state;
    gst_element_get_state(player->pipeline, &state, NULL, GST_CLOCK_TIME_NONE);
    return state;
}

float get_position(const MusicPlayer *player) {
    gint64 position = 0;
    if (gst_element_query_position(player->pipeline, GST_FORMAT_TIME, &position)) {
        return (float)position / GST_SECOND;
    }
    return -1.0;
}

void set_position(MusicPlayer *player, int position) {
    gint64 pos = (gint64)position * GST_SECOND;
    gst_element_seek_simple(player->pipeline, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, pos);
}

float get_duration(const MusicPlayer *player) {
    return get_media_duration(player);
}