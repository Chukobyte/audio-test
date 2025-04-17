cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(audio_test C)

if (NOT TARGET miniaudio)
    add_library(miniaudio INTERFACE thirdparty/miniaudio/miniaudio.h)
    target_include_directories(miniaudio INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/miniaudio")
endif()

add_executable(${PROJECT_NAME}_app src/audio_pthread.c src/main.c)
target_link_libraries(${PROJECT_NAME}_app miniaudio)
