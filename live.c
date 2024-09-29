#include <gst/gst.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declare the callback function
static void on_pad_added(GstElement *src, GstPad *new_pad, GstElement *convert);

// Global variables
GstElement *pipeline, *source, *convert, *resample, *volume_element, *sink;
GMainLoop *loop;
GstBus *bus;
GstMessage *msg;
GError *error = NULL;
char *current_uri = NULL;

// Function declarations
void play_stream(const char *uri);
void pause_stream();
void resume_stream();
void set_volume(int volume);
void next_stream(const char *uri);

int main(int argc, char *argv[]) {
    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create the main loop
    loop = g_main_loop_new(NULL, FALSE);

    // Create the pipeline elements
    source = gst_element_factory_make("uridecodebin", "source");
    convert = gst_element_factory_make("audioconvert", "convert");
    resample = gst_element_factory_make("audioresample", "resample");
    volume_element = gst_element_factory_make("volume", "volume");
    sink = gst_element_factory_make("autoaudiosink", "sink");

    // Check if all elements are created successfully
    if (!source || !convert || !resample || !volume_element || !sink) {
        g_printerr("Failed to create one or more GStreamer elements.\n");
        return -1;
    }

    // Create a new pipeline
    pipeline = gst_pipeline_new("audio-player");

    if (!pipeline) {
        g_printerr("Failed to create the GStreamer pipeline.\n");
        return -1;
    }

    // Add all elements to the pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, convert, resample, volume_element, sink, NULL);

    // Link the elements (source is linked dynamically in uridecodebin's pad-added callback)
    if (!gst_element_link(convert, resample) || !gst_element_link(resample, volume_element) || !gst_element_link(volume_element, sink)) {
        g_printerr("Failed to link the pipeline elements.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // Connect the pad-added signal to handle dynamic pads in uridecodebin
    g_signal_connect(source, "pad-added", G_CALLBACK(on_pad_added), convert);

    // Start the main loop in a separate thread
    g_thread_new("main-loop", (GThreadFunc)g_main_loop_run, loop);

    // User input loop
    char command[256];
    while (1) {
        printf("Enter command (play <url>, pause, resume, volume <0-100>, next <url>, quit): ");
        fgets(command, sizeof(command), stdin);

        if (strncmp(command, "play ", 5) == 0) {
            char *url = command + 5;
            url[strcspn(url, "\n")] = '\0'; // Remove newline character
            play_stream(url);
        } else if (strcmp(command, "pause\n") == 0) {
            pause_stream();
        } else if (strcmp(command, "resume\n") == 0) {
            resume_stream();
        } else if (strncmp(command, "volume ", 7) == 0) {
            int volume = atoi(command + 7);
            set_volume(volume);
        } else if (strncmp(command, "next ", 5) == 0) {
            char *url = command + 5;
            url[strcspn(url, "\n")] = '\0'; // Remove newline character
            next_stream(url);
        } else if (strcmp(command, "quit\n") == 0) {
            break;
        } else {
            printf("Unknown command\n");
        }
    }

    // Clean up
    g_main_loop_quit(loop);
    g_main_loop_unref(loop);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}

void play_stream(const char *uri) {
    if (current_uri) {
        free(current_uri);
    }
    current_uri = strdup(uri);
    g_object_set(source, "uri", current_uri, NULL);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void pause_stream() {
    gst_element_set_state(pipeline, GST_STATE_PAUSED);
}

void resume_stream() {
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void set_volume(int volume) {
    gdouble vol = (gdouble)volume / 100.0;
    g_object_set(volume_element, "volume", vol, NULL);
}

void next_stream(const char *uri) {
    gst_element_set_state(pipeline, GST_STATE_NULL);
    play_stream(uri);
}

// Callback for dynamically linking uridecodebin's src pad to the next element
static void on_pad_added(GstElement *src, GstPad *new_pad, GstElement *convert) {
    GstPad *sink_pad = gst_element_get_static_pad(convert, "sink");

    if (gst_pad_is_linked(sink_pad)) {
        g_print("Pad is already linked. Ignoring...\n");
        goto exit;
    }

    // Try to link the pads
    if (gst_pad_link(new_pad, sink_pad) != GST_PAD_LINK_OK) {
        g_printerr("Failed to link uridecodebin to audioconvert.\n");
    } else {
        g_print("Pads linked successfully.\n");
    }

exit:
    gst_object_unref(sink_pad);
}

// gcc live.c -o live `pkg-config --cflags --libs gstreamer-1.0 glib-2.0`

// https://stream.zeno.fm/kzd2e3tx24zuv
// https://stream.zeno.fm/gtpfzu9bfwzuv
