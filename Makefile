all: buildso testtest runtest
 
buildso:
	gcc -shared -fPIC -o libmusicplayer.so music_player.c -lSDL2 -lSDL2_mixer

testtest:
	gcc -o test test.c -L. -lSDL2 -lSDL2_mixer -lmusicplayer  -Wl,-rpath,'.'

runtest:
	./test
