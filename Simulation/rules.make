
BUILD_CONFIGURATIONS = debug release profile
DEFAULT_CONFIGURATION = debug

SOURCE_DIR = ./src
DOCS_DIR = ./docs

LOCAL_INCLUDES += \
	$(shell find $(SOURCE_DIR) -type d) \
	third_party/include/freetype2 \
	third_party/include

LOCAL_LIBS += \
	third_party/lib \
	third_party/lib64

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
	z \
	bz2 \
	GL \
	GLU \
	:libGLEW.a \
	:libglfw3.a dl X11 Xcursor Xrandr Xext Xxf86vm Xinerama \
	:libfreetype.a harfbuzz \
	:libpng16.a \
	:libtga.a \
	:libturbojpeg.a \
	:libtinyobjloader.a \
	boost_system \
	boost_filesystem \
	boost_serialization

# LINKFLAGS += $(shell pkg-config --static --libs glfw3)

PCHCPP = ste/engine/stdafx.hpp

SOURCES := $(shell find $(SOURCE_DIR) -iname "*.c")
SOURCES += $(shell find $(SOURCE_DIR) -iname "*.cpp")
HEADERS := $(shell find $(SOURCE_DIR) -iname "*.h")
HEADERS += $(shell find $(SOURCE_DIR) -iname "*.hpp")

COMPILE = g++
LINK 	= g++

TARGET = simulation

DOXYGEN_CONFIG = docs/doxyconfig
