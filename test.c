#include "music_player.h"
#include <stdio.h>

int main() {
    if (initialize_music_player() != 0) {
        return -1;
    }

    MusicPlayer player;
    load_music(&player, "/media/ragu/Ragu G/Users/91759/Music/IV_songs/Madura-Palapalakkuthu-MassTamilan.org.mp3");
    set_volume(&player, MIX_MAX_VOLUME / 10);
    play_music(&player);

    cleanup_music_player();
    return 0;
}
