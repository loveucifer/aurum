SDL_INCLUDE = -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -I/opt/homebrew/opt/lua@5.4/include/lua5.4 -I./lib/lua -I./lib/imgui -I./lib/imgui/backends
SDL_LIBS    = -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer /opt/homebrew/opt/lua@5.4/lib/liblua.5.4.dylib

IMGUI_SRC   = ./lib/imgui/imgui.cpp \
              ./lib/imgui/imgui_draw.cpp \
              ./lib/imgui/imgui_tables.cpp \
              ./lib/imgui/imgui_widgets.cpp \
              ./lib/imgui/backends/imgui_impl_sdl2.cpp \
              ./lib/imgui/backends/imgui_impl_sdlrenderer2.cpp

build:
	g++ -w -std=c++17 -Wfatal-errors $(SDL_INCLUDE) \
	./src/*.cpp $(IMGUI_SRC) \
	-o aurum \
	$(SDL_LIBS)

clean:
	rm ./aurum;

run:
	./aurum;
