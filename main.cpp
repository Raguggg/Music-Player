#include "music_player.h"
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Progress.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Box.H>
#include <cstdio>
#include <thread>
#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <memory>
#include <cstdlib>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;


// Custom colors
#define BLUE fl_rgb_color(0x42, 0xA5, 0xF5)
#define SEL_BLUE fl_rgb_color(0x21, 0x96, 0xF3)
#define GRAY fl_rgb_color(0x75, 0x75, 0x75)
#define LIGHT_GRAY fl_rgb_color(211, 211, 211)
#define BLACK fl_rgb_color(0, 0, 0)

// diclare get_next_song
std::string get_next_song(std::ifstream& song_file);

// Struct to hold player and progress bar
struct PlayerData {
    MusicPlayer* player;
    Fl_Progress* progress;
    std::ifstream* url_file;
    std::string current_song_path;
    int volume = 50;
    // next is bool
    bool next = false;
};

// Struct to hold PlayerData and volume label
struct VolumeData {
    PlayerData* pdata;
    Fl_Box* volume_label;
};

// Callback function for buttons
void button_cb(Fl_Widget* widget, void* window) {
    ((Fl_Window*)window)->hide();
}

void next_button_cb(Fl_Widget* widget, void* data) {
    // Set the next flag to true
    PlayerData* pdata = (PlayerData*)data;
    pdata->next = true;


}

// Play/pause callback
void play_pause_cb(Fl_Widget* widget, void* data) {
    Fl_Button* button = (Fl_Button*)widget;
    PlayerData* pdata = (PlayerData*)data;
    MusicPlayer* player = pdata->player;
    if (strcmp(button->label(), "▐▐") == 0) {
        button->label("▶");
        pause_music(player);
    } else {
        button->label("▐▐");
        resume_music(player);
    }
}

// Callback function for the volume slider
void volume_cb(Fl_Widget* widget, void* data) {
    Fl_Slider* slider = (Fl_Slider*)widget;
    VolumeData* vdata = (VolumeData*)data;
    PlayerData* pdata = vdata->pdata;
    MusicPlayer* player = pdata->player;
    Fl_Box* label = vdata->volume_label;
    int volume = (int)slider->value();
    label->copy_label(std::to_string(volume).c_str());

    // printf("Volume: %d\n", volume);
    set_volume(player, volume);
    pdata->volume = volume;
}

std::string download_next_song(std::ifstream& url_file) {
    std::string url;
    if (std::getline(url_file, url)) {
        // Create a temporary directory
        const char* tempDir = "/tmp/CPP_AUDIO";
        const char* createTmpCmd = "mkdir -p /tmp/CPP_AUDIO";
        // Clean the directory
        const char* cleanTmpCmd = "rm -rf /tmp/CPP_AUDIO/*";
        system(createTmpCmd);
        system(cleanTmpCmd);
        // get current time in unix timestamp as string
        char timestamp[20];
        std::time_t t = std::time(nullptr);
        std::strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", std::localtime(&t));
        printf("Timestamp: %s\n", timestamp);
        printf("Created temporary directory: %s\n", tempDir);
        // Construct the yt-dlp command, replacing title with timestamp bestaudio/best -vn -ar 44100 -ac 2 -b:a 192k
        std::string command = "yt-dlp -f worstaudio  --output \"" + std::string(tempDir) + "/" + timestamp + ".%(ext)s\" "
                            + "--exec \"ffmpeg -i \\\"" + std::string(tempDir) + "/" + timestamp + ".%(ext)s\\\" "
                            + "-vn -ar 16000 -ac 1 -b:a 32k -bufsize 64k \\\"" + std::string(tempDir) + "/" + timestamp + ".mp3\\\"\" "
                            + "--exec \"rm \\\"" + std::string(tempDir) + "/" + timestamp + ".%(ext)s\\\"\" "
                            + " -- " + url;

        printf("Downloading song: %s\n", command.c_str());
        system(command.c_str());
        printf("Downloaded song\n");

        // Find the downloaded file
        std::string song_path = std::string(tempDir) + "/";
        printf("Finding song in: %s\n", song_path.c_str());
        std::string find_command = "find " + song_path + " -type f -name '*.mp3'";
        FILE* pipe = popen(find_command.c_str(), "r");
        if (!pipe) {
            perror("popen");
            return "";
        }
        char buffer[128];
        std::string result = "";
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            result += buffer;
        }
        pclose(pipe);

        // Remove newline character from the result
        result.erase(result.find_last_not_of(" \n\r\t") + 1);

        return result;
    }
    return "";
}

void play_song(PlayerData *pdata) {
    MusicPlayer* player = pdata->player;
    Fl_Progress* progress = pdata->progress;
    std::ifstream& url_file = *(pdata->url_file);

    while (true) {
        // break;
        // std::string song_path = download_next_song(url_file);
        std::string song_path = get_next_song(url_file);
        printf("Song path: %s\n", song_path.c_str());
        if (song_path.empty()) {
            break;
        }
        pdata->current_song_path = song_path;

        printf("Initializing music player\n");
        if (initialize_music_player() != 0) {
            fprintf(stderr, "Failed to initialize music player\n");
            return;
        }
        set_volume(player, pdata->volume);
        printf("Volume: %d\n", get_volume(player));
        printf("Loading music path: %s\n", song_path.c_str());
        load_music(player, song_path.c_str());
        printf("Music loaded\n");
        play_music(player);
        while (get_status(player)) {
            SDL_Delay(1000);

            // Update progress bar
            progress->value(get_position(player) / get_duration(player));
            // printf("Position: %f\n", get_position(player));
            // printf("Duration: %f\n", get_duration(player));
            Fl::check(); // Update the GUI
            // if next button is pressed, break the loop
            if (pdata->next) {
                pdata->next = false;
                // call stop music to stop the current song
                stop_music(player);
                break;
            }
        }
        cleanup_music_player();

        // Remove the current song file
        // if (std::remove(song_path.c_str()) != 0) {
        //     fprintf(stderr, "Failed to remove file: %s\n", song_path.c_str());
        // }
        reset_music_player();
    }
}

std::string get_next_song(std::ifstream& song_file) {
    std::string path;
    if (std::getline(song_file, path)) {
        return path;
    }
    return "";
}


int main(int argc, char **argv) {
    // Create a window
    Fl_Window *window = new Fl_Window(600, 400, "Music Player");
    window->color(BLACK);

    // Initialize the music player
    MusicPlayer player;

    // Progress bar with smooth appearance
    Fl_Progress *progress = new Fl_Progress(50, 250, 500, 40);
    progress->minimum(0);
    progress->maximum(1);
    progress->value(0);
    progress->color(LIGHT_GRAY);
    progress->selection_color(BLUE);

    // Open the URL file
    // std::ifstream url_file("urls.txt");
    // if (!url_file.is_open()) {
    //     perror("Failed to open urls.txt");
    //     return 1;
    // }
    std::ofstream song_paths("mp3_paths.txt");
    
    // Iterate through the directory to find all .mp3 files and store them in mp3_paths.txt
    std::string folder_path = "/myfolder";
    for (const auto& entry : fs::directory_iterator(folder_path)) {
        if (entry.path().extension() == ".mp3") {
            song_paths << entry.path().string() << std::endl;
        }
    }
    song_paths.close();
    
    // Open the mp3_paths.txt file for lazy loading
    std::ifstream mp3_file("mp3_paths.txt");
    if (!mp3_file.is_open()) {
        std::cerr << "Failed to open mp3_paths.txt" << std::endl;
        return 1;
    }

    // Struct to hold player and progress bar
    PlayerData pdata = { &player, progress, &mp3_file, "" };

    // Create a thread to play the song
    std::thread play_thread(play_song, &pdata);
    play_thread.detach();

    // Volume control slider
    Fl_Slider *volume_slider = new Fl_Slider(50, 50, 500, 30);
    volume_slider->type(FL_HOR_NICE_SLIDER);
    volume_slider->bounds(0, 100);
    volume_slider->value(pdata.volume);
    volume_slider->color(LIGHT_GRAY);
    volume_slider->selection_color(BLUE);
    // add some UI design to the slider
    volume_slider->slider(FL_FLAT_BOX);
    volume_slider->color2(GRAY);
    volume_slider->selection_color(BLUE);
    volume_slider->labelcolor(FL_WHITE);
    volume_slider->labelfont(FL_HELVETICA_BOLD);
    volume_slider->labelsize(20);

    
    // Create a label next to the volume slider to show the volume percentage
    Fl_Box *volume_label = new Fl_Box(10, 50, 40, 40, "50");
    volume_label->labelsize(20);
    volume_label->labelfont(FL_HELVETICA_BOLD);
    volume_label->labelcolor(FL_WHITE);
    volume_label->color(BLACK);
    // change the location of the volume label to the right of the slider
    volume_label->position(volume_slider->x() + volume_slider->w() + 10, volume_slider->y());

    // Struct to hold PlayerData and volume label
    VolumeData vdata = { &pdata, volume_label };

    volume_slider->callback(volume_cb, &vdata);

    // Buttons with modern flat box style
    Fl_Button *button1 = new Fl_Button(50, 150, 150, 50, "<");
    Fl_Button *button2 = new Fl_Button(225, 150, 150, 50, "▐▐");
    Fl_Button *button3 = new Fl_Button(400, 150, 150, 50, ">");

    button1->color(BLACK);
    button1->selection_color(SEL_BLUE);
    button1->labelcolor(FL_WHITE);
    button1->box(FL_FLAT_BOX);
    button1->labelfont(FL_HELVETICA_BOLD + FL_ITALIC);

    button2->color(BLUE);
    button2->selection_color(SEL_BLUE);
    button2->labelcolor(FL_WHITE);
    button2->box(FL_FLAT_BOX);

    button3->color(BLUE);
    button3->selection_color(SEL_BLUE);
    button3->labelcolor(FL_WHITE);
    button3->box(FL_FLAT_BOX);

    button1->callback(button_cb, window);
    button2->callback(play_pause_cb, &pdata);
    // button3->callback(button_cb, window);
    button3->callback(next_button_cb, &pdata);

    // Set window and overall theme
    Fl::background(255, 255, 255);
    Fl::visible_focus(false);

    window->end();
    window->show(argc, argv);

    return Fl::run();
}
// g++ -o main main.cpp music_player.c -lSDL2 -lSDL2_mixer -lfltk -lpthread