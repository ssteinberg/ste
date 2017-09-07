#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "tinyobjloader" for configuration "Release"
set_property(TARGET tinyobjloader APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(tinyobjloader PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/tinyobjloader.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS tinyobjloader )
list(APPEND _IMPORT_CHECK_FILES_FOR_tinyobjloader "${_IMPORT_PREFIX}/lib/tinyobjloader.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
