cmake_minimum_required(VERSION 2.6)

PROJECT(REGISTRATION)

# ITK
FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

# Boost
FIND_PACKAGE(Boost 1.44 COMPONENTS program_options filesystem system REQUIRED)
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
                    "lib/Builders"
                    "lib/Builders/StackBuilders"
                    "lib/StackAligners"
                    "lib/ImageFunctions"
                   )

ADD_SUBDIRECTORY(lib)

# Targets
ADD_EXECUTABLE(ShrinkImage ShrinkImage.cxx )
TARGET_LINK_LIBRARIES(ShrinkImage ${ITK_LIBRARIES})

ADD_EXECUTABLE(FlipImage FlipImage.cxx )
TARGET_LINK_LIBRARIES(FlipImage ${ITK_LIBRARIES})

ADD_EXECUTABLE(RotateImage RotateImage.cxx )
TARGET_LINK_LIBRARIES(RotateImage ${ITK_LIBRARIES} ${Boost_LIBRARIES})

ADD_EXECUTABLE(IntensifyImage IntensifyImage.cxx )
TARGET_LINK_LIBRARIES(IntensifyImage ${ITK_LIBRARIES} ${Boost_LIBRARIES})

ADD_EXECUTABLE(RegisterVolumes RegisterVolumes.cxx )
TARGET_LINK_LIBRARIES(RegisterVolumes ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                   Dirs Parameters)

ADD_EXECUTABLE(BuildColourVolume BuildColourVolume.cxx )
TARGET_LINK_LIBRARIES(BuildColourVolume ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                   Dirs Parameters)

ADD_EXECUTABLE(DeformableRegistration DeformableRegistration.cxx )
TARGET_LINK_LIBRARIES(DeformableRegistration ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                  Dirs Parameters)

ADD_EXECUTABLE(ConvertRGBAToRGB ConvertRGBAToRGB.cxx )
TARGET_LINK_LIBRARIES(ConvertRGBAToRGB ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                  Dirs)

ADD_EXECUTABLE(ComposeTransformSeries ComposeTransformSeries.cxx )
TARGET_LINK_LIBRARIES(ComposeTransformSeries ${ITK_LIBRARIES} ${Boost_LIBRARIES} Dirs)

ADD_EXECUTABLE(ComposeBananaTransformSeries ComposeBananaTransformSeries.cxx )
TARGET_LINK_LIBRARIES(ComposeBananaTransformSeries ${ITK_LIBRARIES} ${Boost_LIBRARIES} Dirs)

ADD_EXECUTABLE(RegisterROI RegisterROI.cxx )
TARGET_LINK_LIBRARIES(RegisterROI ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                  Dirs Parameters)

ADD_EXECUTABLE(RegisterLoResPair RegisterLoResPair.cxx )
TARGET_LINK_LIBRARIES(RegisterLoResPair ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                  Dirs Parameters)

ADD_EXECUTABLE(RegisterHiResPairs RegisterHiResPairs.cxx )
TARGET_LINK_LIBRARIES(RegisterHiResPairs ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                  Dirs Parameters)

ADD_EXECUTABLE(SplitVolumeIntoSlices SplitVolumeIntoSlices.cxx )
TARGET_LINK_LIBRARIES(SplitVolumeIntoSlices ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                  Dirs)

ADD_EXECUTABLE(TestPrincipalAxes TestPrincipalAxes.cxx )
TARGET_LINK_LIBRARIES(TestPrincipalAxes ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                  Dirs Parameters)

ADD_EXECUTABLE(ImageStats ImageStats.cxx )
TARGET_LINK_LIBRARIES(ImageStats ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                  Dirs Parameters)

ADD_EXECUTABLE(BuildProgressVolume BuildProgressVolume.cxx )
TARGET_LINK_LIBRARIES(BuildProgressVolume ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                  Dirs Parameters)

ADD_EXECUTABLE(ComputeDiffusionTransforms ComputeDiffusionTransforms.cxx )
TARGET_LINK_LIBRARIES(ComputeDiffusionTransforms ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                  Dirs Parameters)

ADD_EXECUTABLE(GenerateNoisyTransforms GenerateNoisyTransforms.cxx )
TARGET_LINK_LIBRARIES(GenerateNoisyTransforms ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                  Dirs Parameters)

ADD_EXECUTABLE(ConvertImage ConvertImage.cxx )
TARGET_LINK_LIBRARIES(ConvertImage ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                  Dirs Parameters)

ADD_EXECUTABLE(RemoveLabelObjects RemoveLabelObjects.cxx )
TARGET_LINK_LIBRARIES(RemoveLabelObjects ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                  Dirs Parameters)

ADD_EXECUTABLE(CropImage CropImage.cxx )
TARGET_LINK_LIBRARIES(CropImage ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                  Dirs Parameters)

ADD_EXECUTABLE(BuildRedBlueChannelComparison BuildRedBlueChannelComparison.cxx )
TARGET_LINK_LIBRARIES(BuildRedBlueChannelComparison ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                                  Dirs Parameters)

ADD_SUBDIRECTORY(test)
