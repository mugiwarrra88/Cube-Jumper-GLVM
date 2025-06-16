SHELL := powershell.exe
.SHELLFLAGS := -Command
CXX = C:/msys64/ucrt64/bin/x86_64-w64-mingw32-g++.exe
CC = C:/msys64/ucrt64/bin/x86_64-w64-mingw32-gcc.exe
VULKAN_SDK = C:/msys64/ucrt64
LDFLAGS = -I$(VULKAN_SDK)/include -L$(VULKAN_SDK)/lib -lgdi32 -lopengl32 -lwinmm -lvulkan -static-libgcc -static-libstdc++ -static -mconsole
CXXFLAGS = -c -std=c++20 -g -Wall -I$(VULKAN_SDK)/include -O0 -mconsole
CFLAGS = -c -g -Wall
INC = -I./include
TEX = -I./textures
BUILD = build
SOURCES = ./src/Engine.cpp ./src/EngineMain.cpp GLPointer.c \
	./src/ShaderProgram.cpp ./src/Event.cpp ./src/WinApi/ChronoWin.cpp ./src/TimerCreator.cpp \
	./src/Systems/CollisionSystem.cpp ./src/Systems/AnimationSystem.cpp ./src/Systems/GUISystem.cpp \
	./src/Systems/PhysicsSystem.cpp ./src/ComponentManager.cpp ./src/EntityManager.cpp ./src/Systems/MovementSystem.cpp \
	./src/SystemManager.cpp ./src/Systems/ProjectileSystem.cpp ./src/Systems/CameraSystem.cpp \
	./src/WinApi/SoundEngineWaveform.cpp ./src/SoundEngineFactory.cpp ./src/GraphicAPI/Opengl.cpp ./src/GraphicAPI/Vulkan.cpp ./src/TextureManager.cpp \
	./src/WavefrontObjParser.cpp ./src/MeshManager.cpp ./src/JsonParser.cpp \
	./src/WinApi/WindowWinVulkan.cpp ./src/WinApi/WindowWinOpengl.cpp \
	./textures/glvm.cpp ./textures/sample1.cpp ./textures/sample2.cpp 
OBJECTS = $(SOURCES:./src/%.cpp=$(BUILD)\\%.o)
EXECUTABLE = winGame

all:$(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $(BUILD)/$@

$(BUILD)\\%.o : ./src/%.cpp
	New-Item -Force -ItemType Directory -Path $(@D)
	$(CXX) $(INC) $(TEX) $(CXXFLAGS) $< -o $@

$(BUILD)\\%.o : %.c
	$(CC) $(INC) $(TEX) $(CFLAGS) $< -o $@

clean:
	Remove-Item "$(BUILD)\*" -Recurse

