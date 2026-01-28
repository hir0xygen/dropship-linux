cmake_minimum_required(VERSION 3.16)
project(dropship VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform detection
if(WIN32)
    message(STATUS "Building for Windows - use Visual Studio/MSBuild instead")
    message(FATAL_ERROR "CMake build is for Linux only. Use dropship.sln for Windows builds.")
endif()

if(NOT UNIX)
    message(FATAL_ERROR "This CMake configuration only supports Linux")
endif()

message(STATUS "Building Dropship for Linux")

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(CURL REQUIRED)

pkg_check_modules(GLFW3 REQUIRED glfw3)

# ImGui sources (core)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui-docking)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

# ImGui backends for Linux (GLFW + OpenGL3)
# These files need to be added to vendor/imgui-docking/
set(IMGUI_BACKEND_SOURCES
    ${IMGUI_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/imgui_impl_opengl3.cpp
)

# Shared source files (platform-independent)
# NOTE: Most of these need pch.h refactoring before they can be compiled on Linux
# For now, we only compile the minimal set needed for a working Linux build
set(SHARED_SOURCES
    # These will be added as they are ported to be platform-independent
    # Currently empty - all shared sources depend on pch.h which includes Windows.h
)

# Linux-specific sources
set(LINUX_SOURCES
    src/main_linux.cpp
    src/platform/firewall/firewall_linux.cpp
    src/platform/http/http_linux.cpp
    src/platform/privileges_linux.cpp
)

# Windows-specific sources (not used in CMake build, listed for reference)
set(WINDOWS_ONLY_SOURCES
    src/main.cpp
    src/core/Debug.cpp
    src/util/watcher/process.cpp
    src/util/watcher/window.cpp
    src/util/win/win_download/download_file.cpp
    src/util/win/win_download/download_text.cpp
    src/util/win/win_filesystem/file_picker.cpp
    src/util/win/win_firewall/windows_firewall.cpp
    src/util/win/win_net_fw.cpp
    src/util/win/win_network/windows_network.cpp
)

# Create executable
add_executable(dropship
    ${IMGUI_SOURCES}
    ${IMGUI_BACKEND_SOURCES}
    ${SHARED_SOURCES}
    ${LINUX_SOURCES}
)

# Include directories
target_include_directories(dropship PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    ${IMGUI_DIR}
    ${GLFW3_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

# Compile definitions
target_compile_definitions(dropship PRIVATE
    IMGUI_DEFINE_MATH_OPERATORS
    # Linux-specific defines
    DROPSHIP_LINUX
    # Disable Windows-specific code paths
    NOMINMAX
)

# Link libraries
target_link_libraries(dropship PRIVATE
    ${GLFW3_LIBRARIES}
    OpenGL::GL
    ${CURL_LIBRARIES}
    pthread
)

# Compiler flags
target_compile_options(dropship PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Debug/Release configurations
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(dropship PRIVATE _DEBUG)
    target_compile_options(dropship PRIVATE -g -O0)
else()
    target_compile_options(dropship PRIVATE -O2)
endif()

# Install rules (for AppImage)
install(TARGETS dropship DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../servers.json DESTINATION share/dropship)
# TODO: Install fonts and icons

# Print configuration summary
message(STATUS "")
message(STATUS "Dropship Linux Build Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  GLFW: ${GLFW3_LIBRARIES}")
message(STATUS "  OpenGL: ${OPENGL_LIBRARIES}")
message(STATUS "  CURL: ${CURL_LIBRARIES}")
message(STATUS "")
