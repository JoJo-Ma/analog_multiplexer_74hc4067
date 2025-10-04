cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(analog_multiplexer_74hc4067)



# FILE(GLOB app_sources src/*.cpp)
FILE(GLOB app_sources src/main.cpp src/utils.cpp src/hc4067.cpp)
target_sources(app PRIVATE ${app_sources})



