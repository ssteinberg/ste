
BUILD_CONFIGURATIONS = debug release profile
DEFAULT_CONFIGURATION = debug

SOURCE_DIR = ./src
DOCS_DIR = ./docs

LOCAL_INCLUDES += \
	$(shell find $(SOURCE_DIR) -type d) \
	/usr/include/freetype2 \
	/usr/local/share/icc/compilers_and_libraries/linux/include/

LOCAL_LIBS += \

BUILD_DIR = ./build

CXXFLAGS += \
	-Wall \
	-std=c++14 \
	-D _linux \
	-pthread \
	-w2 \
	-static-intel \
	-diag-error-limit=5 \
	-wd672,10237,11012,11021,10382 \

CXXFLAGS_release += \
	-O3 -ipo -xHOST -no-prec-div -falign-functions=16 -D NDEBUG

CXXFLAGS_profile += \
	$(CXXFLAGS_release) -g

CXXFLAGS_debug += \
	-g -D DEBUG

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
	boost_serialization \
	cgraph \
	gvc \
	imgui

LINKFLAGS += $(shell pkg-config --static --libs glfw3)

PCHCPP = ste/engine/stdafx

SOURCES := $(shell find $(SOURCE_DIR) -iname "*.c")
SOURCES += $(shell find $(SOURCE_DIR) -iname "*.cpp")
HEADERS := $(shell find $(SOURCE_DIR) -iname "*.h")
HEADERS += $(shell find $(SOURCE_DIR) -iname "*.hpp")

COMPILE = icpc
LINK 	= icpc

TARGET = simulation

DOXYGEN_CONFIG = docs/doxyconfig
