# Allows global toolchain to be set using GLOBAL_TOOLCHAIN env
# Note: needs to happen before project is set
if(DEFINED ENV{GLOBAL_TOOLCHAIN})
    message("Set toolchain $ENV{GLOBAL_TOOLCHAIN}")
    set(CMAKE_TOOLCHAIN_FILE $ENV{GLOBAL_TOOLCHAIN})
endif()

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(NukeCGDenoiser)

set(CMAKE_MODULE_PATH "CMake;${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake/Modules")
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

find_package(Nuke REQUIRED)
find_package(OpenImageDenoise REQUIRED)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

function(add_plugin PLUGIN_NAME)
    add_nuke_plugin(${PLUGIN_NAME} ${ARGN})
    target_compile_definitions(${PLUGIN_NAME} PRIVATE FN_PLUGIN)
    target_link_directories(${PLUGIN_NAME} PUBLIC OpenImageDenoise::OpenImageDenoise)
    target_link_libraries(${PLUGIN_NAME} PUBLIC OpenImageDenoise::OpenImageDenoise)
endfunction()

add_plugin(Denoiser src/Denoiser.cpp)