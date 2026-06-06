cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(jotabelike.gd_racha VERSION 1.0.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    DISABLE_PRECOMPILE_HEADERS ON
)

# Install the compiled .geode into a secondary GD installation used for testing
set(EXTRA_GD_MODS_DIR "C:/Users/josue/OneDrive/Escritorio/Geometry Dash 2.2081 - CreceGames/Geometry Dash/geode/mods")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.geode"
        "${EXTRA_GD_MODS_DIR}/${PROJECT_NAME}.geode"
    COMMENT "Installing ${PROJECT_NAME}.geode to test GD installation"
)


