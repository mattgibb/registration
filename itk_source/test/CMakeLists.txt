ADD_EXECUTABLE(Brain Brain.cxx )
TARGET_LINK_LIBRARIES(Brain ${ITKLibs} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                      Dirs Parameters Stack
                      Builders StackAligners)