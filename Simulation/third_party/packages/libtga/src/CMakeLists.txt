SET(LIBTGA_SRCS
    tga.c
    tga.h
    tgaread.c
    tgawrite.c )

ADD_LIBRARY(tga_static STATIC
            ${LIBTGA_SRCS} )
ADD_LIBRARY(tga SHARED
            ${LIBTGA_SRCS} )

TARGET_INCLUDE_DIRECTORIES(tga_static PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}" )
TARGET_INCLUDE_DIRECTORIES(tga PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}" )
                           
