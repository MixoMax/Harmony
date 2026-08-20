cmake_minimum_required(VERSION 3.28.3)
project(Harmony)

cmake_policy(SET CMP0072 NEW)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("Running Setup for Harmony")


#find_package(OpenGL REQUIRED)

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

#fonts
#find_package(Freetype REQUIRED)


if (MINGW)
    set(CMAKE_EXE_LINKER_FLAGS
            "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif ()


add_executable(Harmony
        #        poc/Terminal.cpp
        src/main.cpp
        src/glad.c
        src/settings.h
        src/voiceChannels/VoiceChannelManager.cpp
        src/voiceChannels/VoiceChannelManager.h
        src/voiceChannels/PulseAudioDeviceEnumeration.h
        src/graphics/GraphicsManager.cpp
        src/graphics/GraphicsManager.h
        src/supporters/fileWizard.h
        src/supporters/fileWizard.cpp
        src/supporters/stb_image_impl.cpp
        src/graphics/basicShapes/Rectangle.cpp
        src/graphics/basicShapes/Rectangle.h
        src/supporters/SDF.h
        src/graphics/widgets/FilledButton.cpp
        src/graphics/widgets/FilledButton.h
        src/graphics/widgets/Widget.cpp
        src/graphics/widgets/Widget.h
        src/graphics/meshes/RectangularMesh.cpp
        src/graphics/meshes/RectangularMesh.h
        src/graphics/meshes/MeshInitializer.h
        src/graphics/shaders/ShaderInitializer.h
        src/supporters/InputManager.cpp
        src/supporters/InputManager.h
        src/supporters/interpolation.h
        src/graphics/text/CharacterManager.cpp
        src/graphics/text/CharacterManager.h
        src/supporters/Alignment.h
        src/supporters/Alignment.cpp
        src/voiceChannels/transmissionTechnologies/TransmissionManager.cpp
        src/voiceChannels/transmissionTechnologies/TransmissionManager.h
        src/voiceChannels/transmissionTechnologies/websockets/WebsocketManager.cpp
        src/voiceChannels/transmissionTechnologies/websockets/WebsocketManager.h
        src/voiceChannels/transmissionTechnologies/udpSockets/UDPSocketManager.cpp
        src/voiceChannels/transmissionTechnologies/udpSockets/UDPSocketManager.h
        src/httpUtils/Client.cpp
        src/httpUtils/Client.h
        src/httpUtils/dataClasses/Room.cpp
        src/httpUtils/dataClasses/Room.h
        src/httpUtils/dataClasses/User.cpp
        src/httpUtils/dataClasses/User.h
        src/graphics/pages/Homepage.cpp
        src/graphics/pages/Homepage.h
        src/graphics/pages/Page.cpp
        src/graphics/pages/Page.h
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

add_subdirectory(BasicCppLibrary)

target_link_libraries(Harmony
        #        ${glfwName}
        #        OpenGL::GL
        ${PULSE_LIBRARIES}
        ixwebsocket
        BasicCppLibrary
        #        Freetype::Freetype
)

# Resource Files
add_custom_target(copy-runtime-files ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/res
        DEPENDS Harmony
        COMMENT "Copying resource files to build directory")


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})


