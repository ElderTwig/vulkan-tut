#---------------------------CMake Project Config-------------------------------
cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

project(PROJECT_NAME LANGUAGES CXX)

if(NOT CMAKE_MSVC_RUNTIME_LIBRARY)
    set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(NOT CMAKE_BUILD_TYPE)
    message("Setting build configuration to Release.")
    message("To change, configure with -DCMAKE_BUILD_TYPE=Release/Debug/etc...")

    set(CMAKE_BUILD_TYPE Release CACHE 
        STRING "Sets the configuration to build (Release, Debug, etc...)")
endif()

option(CLANG_TIDY "Run clang-tidy on MandelLandscape" OFF)
option(BUILD_TESTING OFF)
option(BUILD_SHARED_LIBS OFF)

set(APP_NAME "App")

message("App name is: ${APP_NAME}")

#---------------------------Compilation database generation--------------------
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(TOUCH ${CMAKE_BINARY_DIR}/compile_commands.json)
file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json)

configure_file(${CMAKE_BINARY_DIR}/compile_commands.json 
    ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    COPYONLY)

#---------------------------git submodule setup--------------------------------
option(UPDATE_SUBMODULES "Update git submodules before configuration" ON)

if(UPDATE_SUBMODULES)
  include(UpdateSubmodules)
endif()

#---------------------------CCache setup---------------------------------------
find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

#---------------------------Catch2---------------------------------------------
if(BUILD_TESTING)
    find_package(Catch2 QUIET)

    if(NOT Catch2_FOUND)
      add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/Catch2)
    endif()
endif()

#---------------------------APP_NAME::lib--------------------------------------
add_subdirectory(lib)

#---------------------------APP_NAME-------------------------------------------
add_subdirectory(app)

#---------------------------tests----------------------------------------------
if(BUILD_TESTING)
    option(COVERAGE "Run gcovr after testing" ON)
    option(GCOVR_GEN_HTML "Generate html files from coverage" OFF)

    add_subdirectory(tests)
endif()