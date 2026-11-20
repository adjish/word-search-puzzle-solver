cmake_minimum_required(VERSION 3.25)

project(word-search-puzzle-solver CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME} main.cpp)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_STRIP} --strip-all $<TARGET_FILE:${PROJECT_NAME}>)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
