cmake_minimum_required(VERSION 2.6)

PROJECT(REGISTRATION)

# Find ITK.
FIND_PACKAGE(ITK REQUIRED)

FIND_LIBRARY(YAML_LIBRARY
  NAMES libyaml-cpp.dylib libyaml-cpp.so
  PATHS /usr/local/lib $ENV{HOME}/lib
  )

# MESSAGE( STATUS "HOME: $ENV{HOME}" )
ADD_SUBDIRECTORY(lib)
  
SET(ITKLibs ITKCommon ITKBasicFilters ITKIO ITKNumerics)
  
INCLUDE(${ITK_USE_FILE})
# INCLUDE( CMakePrintSystemInformation )

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/Frameworks ${PROJECT_SOURCE_DIR}/Observers /usr/local/include/yaml-cpp $ENV{HOME}/include/yaml-cpp ${PROJECT_SOURCE_DIR}/lib)

ADD_EXECUTABLE(ShrinkImage ShrinkImage.cxx )
TARGET_LINK_LIBRARIES(ShrinkImage ${ITKLibs})

ADD_EXECUTABLE(Register Register.cxx )
TARGET_LINK_LIBRARIES(Register ${ITKLibs} ${YAML_LIBRARY})

ADD_EXECUTABLE(FlipImage FlipImage.cxx )
TARGET_LINK_LIBRARIES(FlipImage ${ITKLibs})

ADD_EXECUTABLE(RotateImage RotateImage.cxx )
TARGET_LINK_LIBRARIES(RotateImage ${ITKLibs})

ADD_EXECUTABLE(BuildVolumes BuildVolumes.cxx )
TARGET_LINK_LIBRARIES(BuildVolumes ${ITKLibs} ${YAML_LIBRARY} lib)

ADD_EXECUTABLE(test test.cpp )
TARGET_LINK_LIBRARIES(test ${ITKLibs} lib)

# GET_TARGET_PROPERTY(LINK_FLAGS REGISTER LINK_FLAGS)
