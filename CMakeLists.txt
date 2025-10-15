cmake_minimum_required(VERSION 3.24)
project(uwurg)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Disable unnecessary Raylib stuff
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)

# Detect Web automatically
if(EMSCRIPTEN)
    set(PLATFORM Web)
else()
    set(PLATFORM Desktop)
endif()

# Basic compiler warnings
if(NOT WIN32)
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Fetch Raylib
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
)
FetchContent_MakeAvailable(raylib)

# Your source files
set(FILE_IN
    src/TextureManager/TextureManager.cpp
    src/ObjectManager/ObjectManager.cpp
    src/SoundManager/SoundManager.cpp
    src/ManagedSound/ManagedSound.cpp
    src/Shaders/ShadersManager.cpp
    src/Window/Window.cpp
    src/Object/Object.cpp
    src/Game/Game.cpp
    src/State/State.cpp
    src/main.cpp
)

# Debug flag
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(DEBUG_MODE)
endif()

# Build target
if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${FILE_IN})
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
else()
    add_executable(${PROJECT_NAME} ${FILE_IN})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
target_include_directories(${PROJECT_NAME} PRIVATE ${raylib_SOURCE_DIR}/src)

# Static link for native builds
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
