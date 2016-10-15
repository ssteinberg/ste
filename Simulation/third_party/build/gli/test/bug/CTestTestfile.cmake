# CMake generated Testfile for 
# Source directory: C:/src/git/StE/Simulation/third_party/packages/gli/test/bug
# Build directory: C:/src/git/StE/Simulation/third_party/build/gli/test/bug
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(test-bug "C:/src/git/StE/Simulation/third_party/build/gli/test/bug/Debug/test-bug.exe")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(test-bug "C:/src/git/StE/Simulation/third_party/build/gli/test/bug/Release/test-bug.exe")
else()
  add_test(test-bug NOT_AVAILABLE)
endif()
