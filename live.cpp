#include <gst/gst.h>
#include <glib.h>
#include <iostream>
#include <string>
#include <thread>
#include <cstring>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Progress.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Box.H>


extern "C" {
    static void on_pad_added(GstElement *src, GstPad *new_pad, GstElement *convert);
}

class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();
    void playStream(const std::string &uri);
    void pauseStream();
    void resumeStream();
    void setVolume(int volume);
    void nextStream(const std::string &uri);
    void run();

private:
    GstElement *pipeline, *source, *convert, *resample, *volumeElement, *sink;
    GMainLoop *loop;
    std::string currentUri;
};

AudioPlayer::AudioPlayer() {
    gst_init(nullptr, nullptr);
    loop = g_main_loop_new(nullptr, FALSE);

    source = gst_element_factory_make("uridecodebin", "source");
    convert = gst_element_factory_make("audioconvert", "convert");
    resample = gst_element_factory_make("audioresample", "resample");
    volumeElement = gst_element_factory_make("volume", "volume");
    sink = gst_element_factory_make("autoaudiosink", "sink");

    if (!source || !convert || !resample || !volumeElement || !sink) {
        std::cerr << "Failed to create one or more GStreamer elements." << std::endl;
        exit(-1);
    }

    pipeline = gst_pipeline_new("audio-player");

    if (!pipeline) {
        std::cerr << "Failed to create the GStreamer pipeline." << std::endl;
        exit(-1);
    }

    gst_bin_add_many(GST_BIN(pipeline), source, convert, resample, volumeElement, sink, nullptr);

    if (!gst_element_link(convert, resample) || !gst_element_link(resample, volumeElement) || !gst_element_link(volumeElement, sink)) {
        std::cerr << "Failed to link the pipeline elements." << std::endl;
        gst_object_unref(pipeline);
        exit(-1);
    }

    g_signal_connect(source, "pad-added", G_CALLBACK(on_pad_added), convert);

    std::thread([this]() { g_main_loop_run(loop); }).detach();
}

AudioPlayer::~AudioPlayer() {
    g_main_loop_quit(loop);
    g_main_loop_unref(loop);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
}

void AudioPlayer::playStream(const std::string &uri) {
    currentUri = uri;
    g_object_set(source, "uri", currentUri.c_str(), nullptr);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void AudioPlayer::pauseStream() {
    gst_element_set_state(pipeline, GST_STATE_PAUSED);
}

void AudioPlayer::resumeStream() {
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void AudioPlayer::setVolume(int volume) {
    gdouble vol = static_cast<gdouble>(volume) / 100.0;
    g_object_set(volumeElement, "volume", vol, nullptr);
}

void AudioPlayer::nextStream(const std::string &uri) {
    gst_element_set_state(pipeline, GST_STATE_NULL);
    playStream(uri);
}

void AudioPlayer::run() {
    std::string command;
    while (true) {
        std::cout << "Enter command (play <url>, pause, resume, volume <0-100>, next <url>, quit): ";
        std::getline(std::cin, command);

        if (command.rfind("play ", 0) == 0) {
            playStream(command.substr(5));
        } else if (command == "pause") {
            pauseStream();
        } else if (command == "resume") {
            resumeStream();
        } else if (command.rfind("volume ", 0) == 0) {
            setVolume(std::stoi(command.substr(7)));
        } else if (command.rfind("next ", 0) == 0) {
            nextStream(command.substr(5));
        } else if (command == "quit") {
            break;
        } else {
            std::cout << "Unknown command" << std::endl;
        }
    }
}

extern "C" {
    static void on_pad_added(GstElement *src, GstPad *new_pad, GstElement *convert) {
        GstPad *sink_pad = gst_element_get_static_pad(convert, "sink");

        if (gst_pad_is_linked(sink_pad)) {
            std::cout << "Pad is already linked. Ignoring..." << std::endl;
            gst_object_unref(sink_pad);
            return;
        }

        if (gst_pad_link(new_pad, sink_pad) != GST_PAD_LINK_OK) {
            std::cerr << "Failed to link uridecodebin to audioconvert." << std::endl;
        } else {
            std::cout << "Pads linked successfully." << std::endl;
        }

        gst_object_unref(sink_pad);
    }
}

int main() {
    AudioPlayer player;
    player.run();
    return 0;
}

// g++ live.cpp -o live `pkg-config --cflags --libs gstreamer-1.0 glib-2.0`