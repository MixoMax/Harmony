cmake_minimum_required(VERSION 3.28.3)
project(Harmony)

cmake_policy(SET CMP0072 NEW)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("Running Setup for Harmony")


find_package(OpenGL REQUIRED)

# audio
find_package(PkgConfig REQUIRED)
pkg_check_modules(PULSE REQUIRED libpulse-simple libpulse)

#websockets
include(FetchContent)
FetchContent_Declare(
        ixwebsocket
        GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
        GIT_TAG v12.0.1
)
set(USE_TLS ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(ixwebsocket)




if (MINGW)
    set(CMAKE_EXE_LINKER_FLAGS
            "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif ()


add_executable(Harmony
        src/main.cpp
#        src/poc.cpp
)

#include directories
if (WIN32)
    set(glfwName "glfw3")
else ()
    set(glfwName "glfw")
endif ()

target_include_directories(Harmony PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libraries/include")
target_include_directories(Harmony PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libraries/lib")
target_include_directories(Harmony PRIVATE ${PULSE_INCLUDE_DIRS})

target_link_libraries(Harmony
        ${glfwName}
        OpenGL::GL
        ${PULSE_LIBRARIES}
        ixwebsocket
)

# Resource Files
add_custom_target(copy-runtime-files ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/res
        DEPENDS Harmony
        COMMENT "Copying resource files to build directory")


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})


