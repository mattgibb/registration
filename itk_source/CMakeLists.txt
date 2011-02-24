cmake_minimum_required(VERSION 2.6)

PROJECT(REGISTRATION)

# ITK
FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})
SET(ITKLibs ITKCommon ITKBasicFilters ITKIO ITKNumerics)  

# Boost
FIND_PACKAGE(Boost 1.45 COMPONENTS program_options REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

# YAML
FIND_LIBRARY(YAML_LIBRARY
  NAMES libyaml-cpp.dylib libyaml-cpp.so
  PATHS /usr/local/lib $ENV{HOME}/lib
  )
  
# Add platform-specific stuff
IF($ENV{HOST})
  SET(HOST $ENV{HOST})
ELSE($ENV{HOST})
  SET(HOST "unknown")
ENDIF($ENV{HOST})

IF(orac STREQUAL ${HOST})
  # INCLUDE_DIRECTORIES(/home/comp-card/mattgibb/orac/include)
ELSEIF(sal STREQUAL ${HOST})
  # INCLUDE_DIRECTORIES(/home/comp-card/mattgibb/sal/include)
ELSE(orac STREQUAL ${HOST})
  INCLUDE_DIRECTORIES(/usr/local/include $ENV{HOME}/include)
ENDIF(orac STREQUAL ${HOST})

# Project tree
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}
                    "lib"
                    "lib/Observers"
                    "lib/RegistrationBuilders"
                    "lib/StackAligners")

ADD_SUBDIRECTORY(lib)

# Targets
ADD_EXECUTABLE(ShrinkImage ShrinkImage.cxx )
TARGET_LINK_LIBRARIES(ShrinkImage ${ITKLibs})

# ADD_EXECUTABLE(Register Register.cxx )
# TARGET_LINK_LIBRARIES(Register ${ITKLibs} ${YAML_LIBRARY})

ADD_EXECUTABLE(FlipImage FlipImage.cxx )
TARGET_LINK_LIBRARIES(FlipImage ${ITKLibs})

ADD_EXECUTABLE(RotateImage RotateImage.cxx )
TARGET_LINK_LIBRARIES(RotateImage ${ITKLibs})

ADD_EXECUTABLE(BuildVolumes BuildVolumes.cxx )
TARGET_LINK_LIBRARIES(BuildVolumes ${ITKLibs} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                   Dirs RegistrationParameters Stack
                                   RegistrationBuilders StackAligners)

ADD_EXECUTABLE(ConvertRGBAToRGB ConvertRGBAToRGB.cxx )
TARGET_LINK_LIBRARIES(ConvertRGBAToRGB ${ITKLibs} ${YAML_LIBRARY} 
                                  Dirs)

ADD_SUBDIRECTORY(test)
