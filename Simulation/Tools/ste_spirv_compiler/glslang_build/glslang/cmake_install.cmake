# Install script for directory: D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang

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
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang_build/glslang/Debug/glslangd.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang_build/glslang/Release/glslang.lib")
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Public" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/Public/ShaderLang.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/Include/arrays.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/Include/BaseTypes.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/Include/Common.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/Include/ConstantUnion.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/Include/InfoSink.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/Include/InitializeGlobals.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/Include/intermediate.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/Include/PoolAlloc.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/Include/ResourceLimits.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/Include/revision.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/Include/ShHandle.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/Include/Types.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/MachineIndependent/glslang_tab.cpp.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/MachineIndependent/gl_types.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/MachineIndependent/Initialize.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/MachineIndependent/iomapper.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/MachineIndependent/LiveTraverser.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/MachineIndependent/localintermediate.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/MachineIndependent/ParseHelper.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/MachineIndependent/reflection.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/MachineIndependent/RemoveTree.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/MachineIndependent/Scan.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/MachineIndependent/ScanContext.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/MachineIndependent/SymbolTable.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/MachineIndependent/Versions.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/MachineIndependent/parseVersions.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/MachineIndependent/propagateNoContraction.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent/preprocessor" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/MachineIndependent/preprocessor/PpContext.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent/preprocessor" TYPE FILE FILES "D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang/glslang/MachineIndependent/preprocessor/PpTokens.h")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/src/git/StE/Simulation/Tools/ste_spirv_compiler/glslang_build/glslang/OSDependent/Windows/cmake_install.cmake")

endif()

