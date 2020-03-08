cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 17)

project(mp3test)

find_library(MP3LAME mp3lame)

find_package(Threads)

add_library(MP3LAME STATIC IMPORTED)

add_executable(mp3test main.cpp)

target_link_libraries(mp3test ${MP3LAME} ${CMAKE_THREAD_LIBS_INIT})

