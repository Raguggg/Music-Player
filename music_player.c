#include "music_player.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Static variables for music position tracking
static Uint64 music_pos = 0;
static long music_pos_time = -1;
static int music_frequency = 0;
static Uint16 music_format = 0;
static int music_channels = 0;

// Callback function for music position tracking
static void mixmusic_callback(void *udata, Uint8 *stream, int len) {
    if (!Mix_PausedMusic()) {
        music_pos += len;
        music_pos_time = SDL_GetTicks();
    }
}

// Function definitions
int initialize_music_player() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "Mix_OpenAudio Error: %s\n", Mix_GetError());
        SDL_Quit();
        return -1;
    }
    return 0;
}

void cleanup_music_player() {
    Mix_CloseAudio();
    SDL_Quit();
}

void load_music(MusicPlayer *player, const char *sound_path) {
    printf("Loading music: %s\n", sound_path);
    player->sound_path = strdup(sound_path);
    player->music = Mix_LoadMUS(player->sound_path);
    if (!player->music) {
        fprintf(stderr, "Failed to load music: %s\n", Mix_GetError());
    }
}

float get_song_length(Mix_Chunk *chunk) {
    int mixerbytes, numsamples;

    if (music_format == AUDIO_S8 || music_format == AUDIO_U8)
        mixerbytes = 1; // 8-bit audio
    else if (music_format == AUDIO_F32LSB || music_format == AUDIO_F32MSB)
        mixerbytes = 4; // 32-bit float audio
    else
        mixerbytes = 2; // 16-bit audio or others

    numsamples = chunk->alen / mixerbytes / music_channels;
    return (float)numsamples / (float)music_frequency;
}

float get_media_duration(const MusicPlayer *player) {
    Mix_Chunk *chunk = Mix_LoadWAV(player->sound_path);
    if (chunk == NULL) {
        fprintf(stderr, "Failed to load sound: %s\n", Mix_GetError());
        return -1;
    }

    float song_length = get_song_length(chunk);
    Mix_FreeChunk(chunk);
    return song_length;
}

void play_music(MusicPlayer *player) {
    Mix_SetPostMix(mixmusic_callback, NULL);
    Mix_QuerySpec(&music_frequency, &music_format, &music_channels);
    player->duration = get_media_duration(player);
    Mix_PlayMusic(player->music, 1);
    while (Mix_PlayingMusic()) {
        SDL_Delay(1000);
        if (get_position(player) >= get_duration(player)) {
            stop_music(player);
        }
    }
}

void pause_music(MusicPlayer *player) {
    Mix_PauseMusic();
}

void resume_music(MusicPlayer *player) {
    Mix_ResumeMusic();
}

void stop_music(MusicPlayer *player) {
    Mix_HaltMusic();
}

void set_volume(MusicPlayer *player, int volume) {
    player->volume = volume;
    Mix_VolumeMusic(player->volume);
}

int get_volume(const MusicPlayer *player) {
    return player->volume;
}

int get_status(const MusicPlayer *player) {
    return player->status;
}

float get_position(const MusicPlayer *player) {
    long ticks;
    ticks = (long)(1000 * music_pos /
                    (music_channels * music_frequency *
                        ((music_format & 0xff) >> 3)));
    if (!Mix_PausedMusic())
        ticks += SDL_GetTicks() - music_pos_time;

    return (float)ticks / 1000;
}

void set_position(MusicPlayer *player, int position) {
    player->position = position;
    Mix_SetMusicPosition(player->position);
}

float get_duration(const MusicPlayer *player) {
    return player->duration;
}
