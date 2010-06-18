cmake_minimum_required(VERSION 2.6)

PROJECT(REGISTRATION)

# Find ITK.
FIND_PACKAGE(ITK REQUIRED)

FIND_LIBRARY(YAML_LIBRARY
  NAMES libyaml-cpp.dylib libyaml-cpp.so
  PATHS /usr/local/lib $ENV{HOME}/lib
  )

# MESSAGE( STATUS "HOME: $ENV{HOME}" )
  
INCLUDE(${ITK_USE_FILE})
# INCLUDE( CMakePrintSystemInformation )

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/Frameworks ${PROJECT_SOURCE_DIR}/Observers /usr/local/include/yaml-cpp $ENV{HOME}/include/yaml-cpp)

ADD_EXECUTABLE(ShrinkImage ShrinkImage.cxx )
TARGET_LINK_LIBRARIES(ShrinkImage ITKCommon ITKBasicFilters ITKIO)

ADD_EXECUTABLE(Register Register.cxx )
TARGET_LINK_LIBRARIES(Register ITKCommon ITKBasicFilters ITKIO ITKNumerics ${YAML_LIBRARY})

ADD_EXECUTABLE(FlipImage FlipImage.cxx )
TARGET_LINK_LIBRARIES(FlipImage ITKCommon ITKBasicFilters ITKIO)

ADD_EXECUTABLE(BuildVolumes BuildVolumes.cxx )
TARGET_LINK_LIBRARIES(BuildVolumes ITKCommon ITKBasicFilters ITKIO ITKNumerics ${YAML_LIBRARY})

ADD_EXECUTABLE(test test.cpp )
TARGET_LINK_LIBRARIES(test ITKCommon ITKBasicFilters ITKIO)

# GET_TARGET_PROPERTY(LINK_FLAGS REGISTER LINK_FLAGS)
