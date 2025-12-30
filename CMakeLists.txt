cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "word-search-puzzle-solver")
project(${PROJECT_NAME})
add_executable(${PROJECT_NAME} main.cpp)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_STRIP} --strip-all ${PROJECT_NAME})
endif()

install(TARGETS ${PROJECT_NAME})
