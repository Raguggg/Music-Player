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
#include <FL/Fl_Choice.H>
#include <FL/Fl_Box.H>
#include <map>
#include <fstream>
#include <sstream>

// Custom colors
#define BLUE fl_rgb_color(0x42, 0xA5, 0xF5)
#define SEL_BLUE fl_rgb_color(0x21, 0x96, 0xF3)
#define GRAY fl_rgb_color(0x75, 0x75, 0x75)
#define LIGHT_GRAY fl_rgb_color(211, 211, 211)
#define BLACK fl_rgb_color(0, 0, 0)

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
    std::cout << "playStream called with URI: " << uri << std::endl;
    currentUri = uri;

    // Set pipeline to NULL state to reset it
    gst_element_set_state(pipeline, GST_STATE_NULL);

    // Set the new URI
    g_object_set(source, "uri", currentUri.c_str(), nullptr);

    // Set pipeline to PLAYING state
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cerr << "Failed to set pipeline to PLAYING state." << std::endl;
    } else {
        std::cout << "Pipeline set to PLAYING state." << std::endl;
    }
}

void AudioPlayer::pauseStream() {
    std::cout << "pauseStream called" << std::endl;
    gst_element_set_state(pipeline, GST_STATE_PAUSED);
}

void AudioPlayer::resumeStream() {
    std::cout << "resumeStream called" << std::endl;
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void AudioPlayer::setVolume(int volume) {
    std::cout << "setVolume called with volume: " << volume << std::endl;
    gdouble vol = static_cast<gdouble>(volume) / 100.0;
    g_object_set(volumeElement, "volume", vol, nullptr);
}

void AudioPlayer::nextStream(const std::string &uri) {
    std::cout << "nextStream called with URI: " << uri << std::endl;
    gst_element_set_state(pipeline, GST_STATE_NULL);
    playStream(uri);
}

extern "C" {
    static void on_pad_added(GstElement *src, GstPad *new_pad, GstElement *convert) {
        std::cout << "on_pad_added called" << std::endl;
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

// Struct to hold player and volume label
struct PlayerData {
    AudioPlayer* player;
    Fl_Choice* url_choice;
    Fl_Button* play_pause_button;
    Fl_Slider* volume_slider;
    Fl_Box* volume_label;
    std::map<std::string, std::string> url_map;
};

// Function to load URLs from a file
std::map<std::string, std::string> loadUrlsFromFile(const std::string& filename) {
    std::map<std::string, std::string> url_map;
    std::ifstream infile(filename);
    std::string line;

    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string name, url;
        if (std::getline(iss, name, '=') && std::getline(iss, url)) {
            url_map[name] = url;
        }
    }

    return url_map;
}

// Callback function for the play/pause button
void play_pause_cb(Fl_Widget* widget, void* data) {
    Fl_Button* button = (Fl_Button*)widget;
    PlayerData* pdata = (PlayerData*)data;
    AudioPlayer* player = pdata->player;
    std::cout << "play_pause_cb called, button label: " << button->label() << std::endl;
    if (strcmp(button->label(), "Pause") == 0) {
        button->label("Play");
        player->pauseStream();
    } else {
        button->label("Pause");
        player->nextStream(pdata->url_map[pdata->url_choice->text()]);
    }
}

// Callback function for the volume slider
void volume_cb(Fl_Widget* widget, void* data) {
    Fl_Slider* slider = (Fl_Slider*)widget;
    PlayerData* pdata = (PlayerData*)data;
    AudioPlayer* player = pdata->player;
    Fl_Box* label = pdata->volume_label;
    int volume = (int)slider->value();
    std::cout << "volume_cb called, volume: " << volume << std::endl;
    label->copy_label(std::to_string(volume).c_str());
    player->setVolume(volume);
}

// Callback function for the URL choice dropdown
void url_choice_cb(Fl_Widget* widget, void* data) {
    Fl_Choice* choice = (Fl_Choice*)widget;
    PlayerData* pdata = (PlayerData*)data;
    AudioPlayer* player = pdata->player;
    std::string name = choice->text();
    std::cout << "url_choice_cb called, selected name: " << name << std::endl;
    std::string url = pdata->url_map[name];
    std::cout << "Corresponding URL: " << url << std::endl;
    player->playStream(url);
    pdata->play_pause_button->label("Pause");
}

int main(int argc, char **argv) {
    // Create a window
    Fl_Window *window = new Fl_Window(600, 400, "Audio Player");
    window->color(BLACK);

    // Initialize the audio player
    AudioPlayer player;

    // Load URLs from file
    std::map<std::string, std::string> url_map = loadUrlsFromFile("urls.txt");

    // URL choice dropdown
    Fl_Choice *url_choice = new Fl_Choice(50, 50, 500, 30);

    // Add names to the dropdown
    for (const auto& pair : url_map) {
        url_choice->add(pair.first.c_str());
    }

    // Play/pause button
    Fl_Button *play_pause_button = new Fl_Button(50, 100, 150, 50, "Play");
    play_pause_button->color(BLUE);
    play_pause_button->selection_color(SEL_BLUE);
    play_pause_button->labelcolor(FL_WHITE);
    play_pause_button->box(FL_FLAT_BOX);

    // Volume control slider
    Fl_Slider *volume_slider = new Fl_Slider(50, 200, 500, 30);
    volume_slider->type(FL_HOR_NICE_SLIDER);
    volume_slider->bounds(0, 100);
    volume_slider->value(50);
    volume_slider->color(LIGHT_GRAY);
    volume_slider->selection_color(BLUE);
    volume_slider->slider(FL_FLAT_BOX);
    volume_slider->color2(GRAY);
    volume_slider->selection_color(BLUE);
    volume_slider->labelcolor(FL_WHITE);
    volume_slider->labelfont(FL_HELVETICA_BOLD);
    volume_slider->labelsize(20);

    // Volume label
    Fl_Box *volume_label = new Fl_Box(560, 200, 40, 30, "50");
    volume_label->labelsize(20);
    volume_label->labelfont(FL_HELVETICA_BOLD);
    volume_label->labelcolor(FL_WHITE);
    volume_label->color(BLACK);

    // Struct to hold player and volume label
    PlayerData pdata = { &player, url_choice, play_pause_button, volume_slider, volume_label, url_map };

    // Set callbacks
    play_pause_button->callback(play_pause_cb, &pdata);
    volume_slider->callback(volume_cb, &pdata);
    url_choice->callback(url_choice_cb, &pdata);

    // Set window and overall theme
    Fl::background(255, 255, 255);
    Fl::visible_focus(false);

    window->end();
    window->show(argc, argv);

    return Fl::run();
}

// g++ live.cpp -o live `pkg-config --cflags --libs gstreamer-1.0 glib-2.0` -lfltk -lpthread