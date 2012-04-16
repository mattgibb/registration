ADD_EXECUTABLE(Brain Brain.cxx )
TARGET_LINK_LIBRARIES(Brain ${ITK_LIBRARIES} ${YAML_LIBRARY} ${Boost_LIBRARIES}
                      Dirs Parameters)