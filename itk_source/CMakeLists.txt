cmake_minimum_required(VERSION 2.6)

PROJECT(REGISTRATION)

# ITK
FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})
SET(ITKLibs ITKCommon ITKBasicFilters ITKIO ITKNumerics)  

# Boost
FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

# YAML
FIND_LIBRARY(YAML_LIBRARY
  NAMES libyaml-cpp.dylib libyaml-cpp.so
  PATHS /usr/local/lib $ENV{HOME}/lib
  )
INCLUDE_DIRECTORIES(/usr/local/include/yaml-cpp $ENV{HOME}/include/yaml-cpp)

# Project tree
ADD_SUBDIRECTORY(lib)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}
                    Frameworks
                    Observers
                    lib)

# Targets
ADD_EXECUTABLE(ShrinkImage ShrinkImage.cxx )
TARGET_LINK_LIBRARIES(ShrinkImage ${ITKLibs})

ADD_EXECUTABLE(Register Register.cxx )
TARGET_LINK_LIBRARIES(Register ${ITKLibs} ${YAML_LIBRARY})

ADD_EXECUTABLE(FlipImage FlipImage.cxx )
TARGET_LINK_LIBRARIES(FlipImage ${ITKLibs})

ADD_EXECUTABLE(RotateImage RotateImage.cxx )
TARGET_LINK_LIBRARIES(RotateImage ${ITKLibs})

ADD_EXECUTABLE(BuildVolumes BuildVolumes.cxx )
TARGET_LINK_LIBRARIES(BuildVolumes ${ITKLibs} ${YAML_LIBRARY} ${Boost_LIBRARIES} Dirs)

ADD_EXECUTABLE(test test.cpp )
TARGET_LINK_LIBRARIES(test ${ITKLibs} Dirs)
