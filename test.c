#include "music_player.h"
#include <stdio.h>
#include <stdlib.h>

void play_song(const char *song_path) {
    printf("Initializing music player\n");
    if (initialize_music_player() != 0) {
        return;
    }
    MusicPlayer player;
    set_volume(&player, 50);
    printf("Volume: %d\n", get_volume(&player));
    printf("Loading music path: %s\n", song_path);
    load_music(&player, song_path);
    printf("loaded\n");
    printf("Volume: %d\n", get_volume(&player));
    play_music(&player);
    while (get_status(&player)) {
        SDL_Delay(1000);
        printf("Position: %f\n", get_position(&player));
    }
    cleanup_music_player();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <song_path>\n", argv[0]);
        return 1;
    }

    const char *song_path = argv[1];
    play_song(song_path);
    printf("Playing song: %s\n", song_path);

    // Cleanup before exiting
    cleanup_music_player();
    reset_music_player();

    return 0;
}