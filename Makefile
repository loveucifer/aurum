CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
INCLUDES = -I/opt/homebrew/opt/glfw/include \
           -I/opt/homebrew/opt/glm/include \
           -I/opt/homebrew/opt/molten-vk/libexec/include
LDFLAGS  = -L/opt/homebrew/opt/glfw/lib -lglfw \
           -L/opt/homebrew/opt/molten-vk/lib -lMoltenVK \
           -framework Cocoa -framework IOKit -framework CoreVideo

TARGET = tiphereth

.PHONY: all build run clean

all: build

build: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@ $(LDFLAGS)

run: build
	./$(TARGET)

clean:
	rm -f $(TARGET)
