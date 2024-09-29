#include <gst/gst.h>
#include <stdio.h>

// Function declaration
static void on_pad_added(GstElement *element, GstPad *pad, GstElement *sink);

int main(int argc, char *argv[]) {
    GstElement *pipeline, *source, *decoder, *converter, *resampler, *volume, *sink;
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;

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
    g_object_set(volume, "volume", 0.1, NULL); // Set volume to 50%

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

    printf("this is 1f\n");
    // Wait until error or EOS
    bus = gst_element_get_bus(pipeline);

    printf("this is test2f\n");
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    printf("this is test3\n");
    // Parse message
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
                g_print("End of stream reached.\n");
                break;
            default:
                g_printerr("Unexpected message received.\n");
                break;
        }
        gst_message_unref(msg);
    }

    // Clean up
    gst_object_unref(bus);
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

// gcc gome.c -o gome `pkg-config --cflags --libs gstreamer-1.0`
