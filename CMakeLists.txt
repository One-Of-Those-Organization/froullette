cmake_minimum_required(VERSION 3.24)
project(froullete)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)

if(EMSCRIPTEN)
    set(PLATFORM Web)
else()
    set(PLATFORM Desktop)
endif()

if(NOT WIN32)
    add_compile_options(-Wall -Wextra)
endif()

FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
)
FetchContent_MakeAvailable(raylib)

set(FILE_IN
    src/main.cpp
    src/Object/Object.cpp
)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(DEBUG_MODE)
endif()

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${FILE_IN})
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
else()
    add_executable(${PROJECT_NAME} ${FILE_IN})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
target_include_directories(${PROJECT_NAME} PRIVATE ${raylib_SOURCE_DIR}/src)

if(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
    target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
endif()


if(EMSCRIPTEN)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
    target_link_options(${PROJECT_NAME} PRIVATE
        "-sUSE_GLFW=3"
        "-sASYNCIFY"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sINITIAL_MEMORY=134217728"
        "-sTOTAL_STACK=67108864"
        "-sWASM=1"
        "-sEXIT_RUNTIME=0"
        "-sUSE_WEBGL2=1"
        "-sFULL_ES3=1"
        "--preload-file"
        "assets@/assets"
    )
endif()
