import ctypes
import os

# Load the shared library
lib_path = os.path.abspath('./libmusicplayer.so')
music_player_lib = ctypes.CDLL(lib_path)

# Define the function prototypes
music_player_lib.initialize_music_player.restype = ctypes.c_int
music_player_lib.cleanup_music_player.argtypes = []
music_player_lib.load_music.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
music_player_lib.load_music.restype = None
music_player_lib.get_song_length.argtypes = [ctypes.c_void_p]
music_player_lib.get_song_length.restype = ctypes.c_float
music_player_lib.get_media_duration.argtypes = [ctypes.c_void_p]
music_player_lib.get_media_duration.restype = ctypes.c_float
music_player_lib.play_music.argtypes = [ctypes.c_void_p]
music_player_lib.play_music.restype = None
music_player_lib.pause_music.argtypes = [ctypes.c_void_p]
music_player_lib.pause_music.restype = None
music_player_lib.resume_music.argtypes = [ctypes.c_void_p]
music_player_lib.resume_music.restype = None
music_player_lib.stop_music.argtypes = [ctypes.c_void_p]
music_player_lib.stop_music.restype = None
music_player_lib.set_volume.argtypes = [ctypes.c_void_p, ctypes.c_int]
music_player_lib.set_volume.restype = None
music_player_lib.get_volume.argtypes = [ctypes.c_void_p]
music_player_lib.get_volume.restype = ctypes.c_int
music_player_lib.get_status.argtypes = [ctypes.c_void_p]
music_player_lib.get_status.restype = ctypes.c_int
music_player_lib.get_position.argtypes = [ctypes.c_void_p]
music_player_lib.get_position.restype = ctypes.c_float
music_player_lib.set_position.argtypes = [ctypes.c_void_p, ctypes.c_int]
music_player_lib.set_position.restype = None
music_player_lib.get_duration.argtypes = [ctypes.c_void_p]
music_player_lib.get_duration.restype = ctypes.c_float

# Define a wrapper class for the MusicPlayer
class MusicPlayer(ctypes.Structure):
    _fields_ = [("sound_path", ctypes.c_char_p),
                ("music", ctypes.c_void_p),
                ("volume", ctypes.c_int),
                ("status", ctypes.c_int),
                ("position", ctypes.c_int),
                ("duration", ctypes.c_float)]

def initialize_player():
    result = music_player_lib.initialize_music_player()
    if result != 0:
        raise RuntimeError("Failed to initialize music player")

def cleanup_player():
    music_player_lib.cleanup_music_player()

def load_music(player, sound_path):
    player.sound_path = sound_path.encode('utf-8')
    music_player_lib.load_music(ctypes.byref(player), player.sound_path)

def get_song_length(player):
    return music_player_lib.get_song_length(ctypes.byref(player))

def get_media_duration(player):
    return music_player_lib.get_media_duration(ctypes.byref(player))

def play_music(player):
    music_player_lib.play_music(ctypes.byref(player))

def pause_music(player):
    music_player_lib.pause_music(ctypes.byref(player))

def resume_music(player):
    music_player_lib.resume_music(ctypes.byref(player))

def stop_music(player):
    music_player_lib.stop_music(ctypes.byref(player))

def set_volume(player, volume):
    music_player_lib.set_volume(ctypes.byref(player), volume)

def get_volume(player):
    return music_player_lib.get_volume(ctypes.byref(player))

def get_status(player):
    return music_player_lib.get_status(ctypes.byref(player))

def get_position(player):
    return music_player_lib.get_position(ctypes.byref(player))

def set_position(player, position):
    music_player_lib.set_position(ctypes.byref(player), position)

def get_duration(player):
    return music_player_lib.get_duration(ctypes.byref(player))

if __name__ == '__main__':
    player = MusicPlayer()
    initialize_player()
    load_music(player, "/media/ragu/Ragu G/Users/91759/Music/IV_songs/Madura-Palapalakkuthu-MassTamilan.org.mp3")
    play_music(player)
    print(get_song_length(player))
    print(get_media_duration(player))
    print(get_status(player))
    print(get_position(player))
    print(get_duration(player))
    set_position(player, 50)
    print(get_position(player))
    pause_music(player)
    print(get_status(player))
    resume_music(player)
    print(get_status(player))
    stop_music(player)
    cleanup_player()