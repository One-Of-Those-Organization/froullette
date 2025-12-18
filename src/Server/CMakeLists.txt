cmake_minimum_required(VERSION 3.20)

set(PROJ_NAME froullete-server)

project(${PROJ_NAME} CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-ggdb)

# Include project headers
include_directories(${CMAKE_SOURCE_DIR}/src)

set(SERVER_SOURCES
    Server.cpp
    mongoose.c
)

add_executable(${PROJ_NAME} ${SERVER_SOURCES})
