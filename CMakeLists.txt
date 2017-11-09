cmake_minimum_required(VERSION 2.8.7)
project (jug_or_not)


# This will produce two important CMake variables, CPR_INCLUDE_DIRS and CPR_LIBRARIES
add_subdirectory(cpr)

include_directories(${CPR_INCLUDE_DIRS})
# target_link_libraries(your_target_name ${CPR_LIBRARIES})
