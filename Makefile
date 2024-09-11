all: buildso testtest runtest
 
buildso:
	gcc -shared -fPIC -o libmusicplayer.so music_player.c -lSDL2 -lSDL2_mixer

testtest:
	gcc -o test test.c -L. -lSDL2 -lSDL2_mixer -lmusicplayer  -Wl,-rpath,'.'

runtest:
	./test
# it uses same shared library
runcpp:
	g++ -o main main.cpp -L. -lSDL2 -lSDL2_mixer -lfltk -lmusicplayer  -Wl,-rpath,'.'
	./main

export_executable:
	gcc -c music_player.c -o musicplayer.o
	g++ -o audio_player_v1 main.cpp musicplayer.o -L. -lSDL2 -lSDL2_mixer -lfltk -Wl,-rpath,'.'


setup:
	sudo apt-get install libfltk1.3-dev
	sudo apt-get install libsdl2-dev
	sudo apt-get install libsdl2-mixer-dev


#  g++ -o main main.cpp music_player.c -lSDL2 -lSDL2_mixer -lfltk -lpthread