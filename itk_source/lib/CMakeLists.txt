# Generate project root dir header
SET(PROJECT_ROOT_DIR "${PROJECT_SOURCE_DIR}/../")
CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/ProjectRootDir.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ProjectRootDir.h"
  )
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

# Build libraries
ADD_LIBRARY(Dirs Dirs.cxx)
ADD_LIBRARY(Parameters Parameters.cxx)
ADD_LIBRARY(Stack Stack.cxx)

ADD_SUBDIRECTORY(Builders)
ADD_SUBDIRECTORY(StackAligners)
