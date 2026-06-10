SDL_INCLUDE = -I/opt/homebrew/include -I/opt/homebrew/opt/lua@5.4/include/lua5.4 -I./lib/lua
SDL_LIBS    = -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer /opt/homebrew/opt/lua@5.4/lib/liblua.5.4.dylib

build:
	g++ -w -std=c++17 -Wfatal-errors $(SDL_INCLUDE) \
	./src/*.cpp \
	-o aurum \
	$(SDL_LIBS)

clean:
	rm ./aurum;

run:
	./aurum;
