# Install script for directory: D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/SPIRV

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang_build/SPIRV/Debug/SPIRVd.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang_build/SPIRV/Release/SPIRV.lib")
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang_build/SPIRV/Debug/SPVRemapperd.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang_build/SPIRV/Release/SPVRemapper.lib")
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/SPIRV" TYPE FILE FILES
    "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/SPIRV/bitutils.h"
    "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/SPIRV/spirv.hpp"
    "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/SPIRV/GLSL.std.450.h"
    "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/SPIRV/GLSL.ext.KHR.h"
    "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/SPIRV/GlslangToSpv.h"
    "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/SPIRV/hex_float.h"
    "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/SPIRV/Logger.h"
    "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/SPIRV/SpvBuilder.h"
    "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/SPIRV/spvIR.h"
    "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/SPIRV/doc.h"
    "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/SPIRV/disassemble.h"
    "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/SPIRV/GLSL.ext.AMD.h"
    "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/SPIRV/GLSL.ext.NV.h"
    "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/SPIRV/SPVRemapper.h"
    "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/SPIRV/doc.h"
    )
endif()

