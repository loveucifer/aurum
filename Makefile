SDL_INCLUDE = -I/opt/homebrew/include
SDL_LIBS    = -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

build:
	g++ -w -std=c++14 -Wfatal-errors $(SDL_INCLUDE) \
	./src/*.cpp \
	-o aurum \
	-I"./lib/lua" \
	-L"./lib/lua" \
	-llua \
	$(SDL_LIBS);

clean:
	rm ./aurum;

run:
	./aurum;
