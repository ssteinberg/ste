
BUILD_CONFIGURATIONS = debug release profile
DEFAULT_CONFIGURATION = debug

SOURCE_DIR = ./src
DOCS_DIR = ./docs
THIRD_PARTY_DIR = ./third_party

LOCAL_INCLUDES += \
	$(shell find $(SOURCE_DIR) -type d)

SYSTEM_INCLUDES += \
	$(THIRD_PARTY_DIR)/include/freetype2 \
	$(THIRD_PARTY_DIR)/include

SYSTEM_LIBS += \
	$(THIRD_PARTY_DIR)/lib \
	$(THIRD_PARTY_DIR)/lib64

BUILD_DIR = ./build
BIN_DIR = ./bin

DEBUG_SYMBOLS_FLAGS = -g3 -ggdb

CXXFLAGS += \
	-std=c++17 \
	-D _linux \
	-pthread \
	-pipe \
	-Wall -Wformat=2 -Wcast-align -Wcast-qual -Wwrite-strings

CXXFLAGS_release += \
	-O3 -march=native -flto -falign-functions=16 -falign-loops=16 -D NDEBUG

CXXFLAGS_profile += \
	$(CXXFLAGS_release) $(DEBUG_SYMBOLS_FLAGS)

CXXFLAGS_debug += \
	$(DEBUG_SYMBOLS_FLAGS) -D _DEBUG

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

PCHCPP = ste/engine/stdafx.hpp

SOURCES := $(shell find $(SOURCE_DIR) -iname "*.c")
SOURCES += $(shell find $(SOURCE_DIR) -iname "*.cpp")
HEADERS := $(shell find $(SOURCE_DIR) -iname "*.h")
HEADERS += $(shell find $(SOURCE_DIR) -iname "*.hpp")

COMPILE = g++
LINK 	= g++

TARGET = simulation

DOXYGEN_CONFIG = docs/doxyconfig
