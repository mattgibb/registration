cmake_minimum_required(VERSION 2.6)

# Find ITK.
FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

ADD_EXECUTABLE(ShrinkImage ShrinkImage.cxx )
TARGET_LINK_LIBRARIES(ShrinkImage ITKCommon ITKBasicFilters ITKIO)

ADD_EXECUTABLE(Register Register.cxx )
TARGET_LINK_LIBRARIES(Register ITKCommon ITKBasicFilters ITKIO ITKNumerics)

ADD_EXECUTABLE(test test.cpp )
TARGET_LINK_LIBRARIES(test ITKCommon ITKBasicFilters ITKIO)

ADD_EXECUTABLE(FlipImage FlipImage.cxx )
TARGET_LINK_LIBRARIES(FlipImage ITKCommon ITKBasicFilters ITKIO)
