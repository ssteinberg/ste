# Install script for directory: D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang

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

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang_build/External/cmake_install.cmake")
  include("D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang_build/glslang/cmake_install.cmake")
  include("D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang_build/OGLCompilersDLL/cmake_install.cmake")
  include("D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang_build/StandAlone/cmake_install.cmake")
  include("D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang_build/SPIRV/cmake_install.cmake")
  include("D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang_build/hlsl/cmake_install.cmake")
  include("D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang_build/gtests/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang_build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
