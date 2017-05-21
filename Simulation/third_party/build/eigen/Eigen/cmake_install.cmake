# Install script for directory: D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/Eigen" TYPE FILE FILES
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/Cholesky"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/CholmodSupport"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/Core"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/Dense"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/Eigen"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/Eigenvalues"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/Geometry"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/Householder"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/IterativeLinearSolvers"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/Jacobi"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/LU"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/MetisSupport"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/OrderingMethods"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/PaStiXSupport"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/PardisoSupport"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/QR"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/QtAlignedMalloc"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/SPQRSupport"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/SVD"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/Sparse"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/SparseCholesky"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/SparseCore"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/SparseLU"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/SparseQR"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/StdDeque"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/StdList"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/StdVector"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/SuperLUSupport"
    "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/UmfPackSupport"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include//Eigen" TYPE DIRECTORY FILES "D:/src/git/StE/Simulation/third_party/packages/eigen/Eigen/src" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

