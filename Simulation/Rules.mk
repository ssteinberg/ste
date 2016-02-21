LOCAL_INCLUDES += \
	./ \
	./include \
	/usr/include/freetype2 \

LOCAL_LIBS += \
	./lib \

CXXFLAGS += \
	-Wextra \
	-std=c++14 \
	-D _linux \
	-pthread \
	-static-intel \

CXXRELEASEFLAGS += \
	-O3 -ipo -xHOST -no-prec-div -D NDEBUG

CXXDEBUGFLAGS += \
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
SOURCES += $(shell find $(SOURCE_DIR) -iname "*.cpp")

COMPILE = icpc
LINK 	= icpc

TARGET = simulation.elf

