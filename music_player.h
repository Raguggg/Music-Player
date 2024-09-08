#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#ifdef __cplusplus
extern "C" {
#endif

// MusicPlayer structure definition
typedef struct {
    char* sound_path;
    Mix_Music *music;
    int volume;
    int status;
    int position;
    long ticks;
    float duration;
} MusicPlayer;

// Function declarations
int initialize_music_player();
void cleanup_music_player();
void load_music(MusicPlayer *player, const char *sound_path);
float get_media_duration(const MusicPlayer *player);
void play_music(MusicPlayer *player);
void pause_music(MusicPlayer *player);
void resume_music(MusicPlayer *player);
void stop_music(MusicPlayer *player);
void set_volume(MusicPlayer *player, int volume);
int get_volume(const MusicPlayer *player);
int get_status(const MusicPlayer *player);
float get_position(const MusicPlayer *player);
void set_position(MusicPlayer *player, int position);
float get_duration(const MusicPlayer *player);

#ifdef __cplusplus
}
#endif

#endif // MUSIC_PLAYER_H
