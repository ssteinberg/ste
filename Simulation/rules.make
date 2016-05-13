
BUILD_CONFIGURATIONS = debug release profile
DEFAULT_CONFIGURATION = debug

SOURCE_DIR = ./src
DOCS_DIR = ./docs

LOCAL_INCLUDES += \
	$(shell find $(SOURCE_DIR) -type d) \
	/usr/include/freetype2

LOCAL_LIBS += \

BUILD_DIR = ./build

CXXFLAGS += \
	-std=c++17 \
	-D _linux \
	-pthread \
	-pipe \
	-Wall -Wformat=2 -Wcast-align -Wundef -Wcast-qual -Wwrite-strings

CXXFLAGS_release += \
	-O3 -march=native -flto -falign-functions=16 -falign-loops=16 -D NDEBUG

CXXFLAGS_profile += \
	$(CXXFLAGS_release) -pg -g

CXXFLAGS_debug += \
	-g -D DEBUG

SYSTEM_LIBRARIES += \
	atomic \
	GL \
	GLU \
	GLEW \
	glfw3 \
	freetype \
	png16 \
	tga \
	turbojpeg \
	tinyobjloader \
	boost_system \
	boost_filesystem \
	boost_serialization \
	cgraph \
	gvc \
	imgui

LINKFLAGS += $(shell pkg-config --static --libs glfw3)

PCHCPP = ste/engine/stdafx.hpp

SOURCES := $(shell find $(SOURCE_DIR) -iname "*.c")
SOURCES += $(shell find $(SOURCE_DIR) -iname "*.cpp")
HEADERS := $(shell find $(SOURCE_DIR) -iname "*.h")
HEADERS += $(shell find $(SOURCE_DIR) -iname "*.hpp")

COMPILE = g++
LINK 	= g++

TARGET = simulation

DOXYGEN_CONFIG = docs/doxyconfig
