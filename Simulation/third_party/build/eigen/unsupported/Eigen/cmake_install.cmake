# Install script for directory: D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "../../")
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

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/unsupported/Eigen" TYPE FILE FILES
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/AdolcForward"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/AlignedVector3"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/ArpackSupport"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/AutoDiff"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/BVH"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/EulerAngles"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/FFT"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/IterativeSolvers"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/KroneckerProduct"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/LevenbergMarquardt"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/MatrixFunctions"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/MoreVectorization"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/MPRealSupport"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/NonLinearOptimization"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/NumericalDiff"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/OpenGLSupport"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/Polynomials"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/Skyline"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/SparseExtra"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/SpecialFunctions"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/Splines"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include//unsupported/Eigen" TYPE DIRECTORY FILES "D:/src/git/StE/Simulation/third_party/packages/eigen/unsupported/Eigen/src" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/src/git/StE/Simulation/third_party/build/eigen/unsupported/Eigen/CXX11/cmake_install.cmake")

endif()

