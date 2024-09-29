#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_File_Chooser.H>
#include <iostream>
#include <vector>
#include <string>
#include <dirent.h>
#include <gst/gst.h>
#include "music_player.h"

// Function declarations
void play_callback(Fl_Widget *w, void *data);
void pause_callback(Fl_Widget *w, void *data);
void volume_callback(Fl_Widget *w, void *data);
void update_progress(void *data);
void load_songs_from_folder(const std::string &folder, std::vector<std::string> &songs);

MusicPlayer player;
std::vector<std::string> songs;
size_t current_song_index = 0;
Fl_Progress *progress_bar;
Fl_Slider *volume_slider;
Fl_Button *play_button;
Fl_Button *pause_button;

int main(int argc, char **argv) {
    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create FLTK window
    Fl_Window *window = new Fl_Window(400, 200, "Music Player");
    
    // Create play button
    play_button = new Fl_Button(50, 150, 100, 30, "Play");
    play_button->callback(play_callback);

    // Create pause button
    pause_button = new Fl_Button(250, 150, 100, 30, "Pause");
    pause_button->callback(pause_callback);

    // Create volume slider
    volume_slider = new Fl_Slider(50, 100, 300, 30, "Volume");
    volume_slider->type(FL_HOR_NICE_SLIDER);
    volume_slider->bounds(0, 100);
    volume_slider->value(50);
    volume_slider->callback(volume_callback);

    // Create progress bar
    progress_bar = new Fl_Progress(50, 50, 300, 30);
    progress_bar->minimum(0);
    progress_bar->maximum(100);
    progress_bar->value(0);

    // Show window
    window->end();
    window->show(argc, argv);

    // Load songs from folder
    if (argc > 1) {
        load_songs_from_folder(argv[1], songs);
    }

    // Start FLTK event loop
    return Fl::run();
}

void play_callback(Fl_Widget *w, void *data) {
    if (songs.empty()) return;

    if (current_song_index >= songs.size()) {
        current_song_index = 0;
    }

    load_music(&player, songs[current_song_index].c_str());
    play_music(&player);

    // Update progress bar
    Fl::add_timeout(0.1, update_progress);
}

void pause_callback(Fl_Widget *w, void *data) {
    pause_music(&player);
}

void volume_callback(Fl_Widget *w, void *data) {
    int volume = static_cast<int>(volume_slider->value());
    set_volume(&player, volume);
}

void update_progress(void *data) {
    float duration = get_duration(&player);
    float position = get_position(&player);

    if (duration > 0) {
        progress_bar->value((position / duration) * 100);
    }

    if (get_status(&player) == GST_STATE_PLAYING) {
        Fl::repeat_timeout(0.1, update_progress);
    }
}

void load_songs_from_folder(const std::string &folder, std::vector<std::string> &songs) {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(folder.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string file_name = ent->d_name;
            if (file_name.find(".mp3") != std::string::npos || file_name.find(".wav") != std::string::npos) {
                songs.push_back(folder + "/" + file_name);
            }
        }
        closedir(dir);
    } else {
        perror("Could not open directory");
    }
}