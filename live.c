#include <gst/gst.h>

// Declare the callback function
static void on_pad_added(GstElement *src, GstPad *new_pad, GstElement *convert);

int main(int argc, char *argv[]) {
    GstElement *pipeline, *source, *convert, *resample, *sink;
    GstBus *bus;
    GstMessage *msg;
    GError *error = NULL;

    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create the pipeline elements
    source = gst_element_factory_make("uridecodebin", "source");
    convert = gst_element_factory_make("audioconvert", "convert");
    resample = gst_element_factory_make("audioresample", "resample");
    sink = gst_element_factory_make("autoaudiosink", "sink");

    // Check if all elements are created successfully
    if (!source || !convert || !resample || !sink) {
        g_printerr("Failed to create one or more GStreamer elements.\n");
        return -1;
    }

    // Create a new pipeline
    pipeline = gst_pipeline_new("audio-player");

    if (!pipeline) {
        g_printerr("Failed to create the GStreamer pipeline.\n");
        return -1;
    }

    // Set the URI to the uridecodebin element
    g_object_set(source, "uri", "https://stream-161.zeno.fm/kzd2e3tx24zuv", NULL);

    // Add all elements to the pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, convert, resample, sink, NULL);

    // Link the elements (source is linked dynamically in uridecodebin's pad-added callback)
    if (!gst_element_link(convert, resample) || !gst_element_link(resample, sink)) {
        g_printerr("Failed to link the pipeline elements.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // Connect the pad-added signal to handle dynamic pads in uridecodebin
    g_signal_connect(source, "pad-added", G_CALLBACK(on_pad_added), convert);

    // Start playing the pipeline
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Wait until an error or EOS (End Of Stream) message
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    // Handle messages
    if (msg != NULL) {
        GError *err;
        gchar *debug_info;

        switch (GST_MESSAGE_TYPE(msg)) {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error(msg, &err, &debug_info);
                g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
                g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
                g_clear_error(&err);
                g_free(debug_info);
                break;
            case GST_MESSAGE_EOS:
                g_print("End-Of-Stream reached.\n");
                break;
            default:
                g_printerr("Unexpected message received.\n");
                break;
        }
        gst_message_unref(msg);
    }

    // Free resources
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
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

// gcc live.c -o live `pkg-config --cflags --libs gstreamer-1.0`