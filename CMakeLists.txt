cmake_minimum_required(VERSION 3.22)
project(AngryTodd)
set(CMAKE_CXX_STANDARD 23)
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "" FORCE)

include(cmake/cpm.cmake)

CPMAddPackage(
    NAME JUCE
    GITHUB_REPOSITORY juce-framework/JUCE
    GIT_TAG 8.0.12
    VERSION 8.0.12
    SOURCE_DIR ${LIB_DIR}/JUCE
)

if (MSVC)
    add_compile_options(/Wall /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_subdirectory(plugin)

