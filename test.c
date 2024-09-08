#include "music_player.h"
#include <stdio.h>

int main() {
    

   
    // ./music_ap.mp3,/media/ragu/Ragu G/Users/91759/Music/IV_songs/Madura-Palapalakkuthu-MassTamilan.org.mp3 array
    // create array of songs
    char* song1 = "/media/ragu/Ragu G/Users/91759/Music/IV_songs/Madura-Palapalakkuthu-MassTamilan.org.mp3";
    char* song2 = "./music_ap.mp3";
    play_song(song2);
    printf("Playing song 1\n");
    play_song(song1);
    printf("Playing song 2\n");

    


    return 0;
}
void play_song(char *song_path){
    if (initialize_music_player() != 0) {
        return -1;
    }
    MusicPlayer player;
    set_volume(&player, 50);
    printf("Volume: %d\n", get_volume(&player));
    // load_music(&player, "");
    load_music(&player, song_path);
    printf("loaded\n");
    // set_volume(&player, MIX_MAX_VOLUME / 10);
    printf("Volume: %d\n", get_volume(&player));
    play_music(&player);
    // // sleep for 30 seconds
    // SDL_Delay(10000);
    // // get status
    // printf("Status: %d\n", get_status(&player));
    while (get_status(&player))
    {
       SDL_Delay(1000);
       printf("Position: %f\n", get_position(&player));
    }
        cleanup_music_player();
}