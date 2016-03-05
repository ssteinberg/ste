
BUILD_CONFIGURATIONS = debug release
DEFAULT_CONFIGURATION = debug

LOCAL_INCLUDES += \
	./ \
	/usr/include/freetype2 \

LOCAL_LIBS += \

BUILD_DIR = ./build

CXXFLAGS += \
	-Wall \
	-std=c++14 \
	-D _linux \
	-pthread \
	-w2 \
	-static-intel \
	-wd10237,11012,11021,10382 \

CXXFLAGS_release += \
	-O3 -ipo -xHOST -no-prec-div -D NDEBUG

CXXFLAGS_debug += \
	-g -gdwarf-4 -D DEBUG

SYSTEM_LIBRARIES += \
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
	boost_serialization

PCHCPP = stdafx

SOURCE_DIR = ./
SOURCES := $(shell find $(SOURCE_DIR) -iname "*.c")
SOURCES += $(shell find $(SOURCE_DIR) -iname "*.cpp")
HEADERS := $(shell find $(SOURCE_DIR) -iname "*.h")
HEADERS += $(shell find $(SOURCE_DIR) -iname "*.hpp")

COMPILE = icpc
LINK 	= icpc

TARGET = simulation

