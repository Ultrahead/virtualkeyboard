# Specify the minimum version of CMake required
cmake_minimum_required(VERSION 3.16)

# Define the project name
project(VirtualKeyboard)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)

# Gather all source files from the src directory
file(GLOB SOURCES "src/*.cpp")

# Create the executable target using the source files
add_executable(VirtualKeyboard ${SOURCES})

# Define a preprocessor macro for the resource directory
target_compile_definitions(VirtualKeyboard PRIVATE RESDIR="res")

# Add a post-build command to copy the "res" directory to the build output folder
add_custom_command(TARGET VirtualKeyboard POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/res"
        "$<TARGET_FILE_DIR:VirtualKeyboard>/res"
        COMMENT "Copying resource directory to build folder..."
)

# Windows-specific configuration (MinGW)
if(WIN32)
    # Find the directory containing the MinGW compiler binary
    get_filename_component(MINGW_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)

    # Error out if MinGW binary path cannot be determined
    if(NOT MINGW_BIN)
        message(FATAL_ERROR "Unable to determine MinGW binary path. Ensure 'project()' has been called and a C++ compiler is active.")
    endif()

    message(STATUS "Found MinGW 'bin' folder: ${MINGW_BIN}")

    # Define paths for required DLLs (e.g., libwinpthread-1.dll)
    set(REQUIRED_DLLS "${MINGW_BIN}/libwinpthread-1.dll")
    get_filename_component(COMPILER_PATH ${CMAKE_CXX_COMPILER} DIRECTORY)
    set(MINGW_DLLS "${COMPILER_PATH}/libwinpthread-1.dll")

    # Add a post-build command to copy MinGW runtime DLLs to the output directory
    add_custom_command(TARGET VirtualKeyboard POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${MINGW_DLLS}
            "$<TARGET_FILE_DIR:VirtualKeyboard>"
            COMMENT "Copying MinGW runtime DLLs to output directory..."
    )

    # Statically link libgcc and libstdc++ to avoid dependency on their DLLs
    target_link_options(VirtualKeyboard PRIVATE -static-libgcc -static-libstdc++)

    # Add include directories for SDL2 and its extensions
    target_include_directories(VirtualKeyboard PRIVATE
            ${CMAKE_SOURCE_DIR}/libs/SDL2-2.26.1/include
            ${CMAKE_SOURCE_DIR}/libs/SDL2_image-2.8.8/x86_64-w64-mingw32/include/SDL2
            ${CMAKE_SOURCE_DIR}/libs/SDL2_ttf-2.24.0/x86_64-w64-mingw32/include/SDL2
            ${CMAKE_SOURCE_DIR}/libs/SDL2_gfx-main
    )

    # Add library directories for linking SDL2 and its extensions
    target_link_directories(VirtualKeyboard PRIVATE
            ${CMAKE_SOURCE_DIR}/libs/SDL2-2.26.1/cmake-build-release
            ${CMAKE_SOURCE_DIR}/libs/SDL2_image-2.8.8/x86_64-w64-mingw32/lib
            ${CMAKE_SOURCE_DIR}/libs/SDL2_ttf-2.24.0/x86_64-w64-mingw32/lib
            ${CMAKE_SOURCE_DIR}/libs/SDL2_gfx-main/cmake-build-release
    )

    # Link against MinGW and SDL2 libraries
    target_link_libraries(VirtualKeyboard PRIVATE
            mingw32
            SDL2_gfx
            SDL2_image
            SDL2_ttf
            SDL2main
            SDL2
    )
else() # Non-Windows configuration (Linux/macOS)

    # Use PkgConfig to find installed libraries
    find_package(PkgConfig REQUIRED)

    # Check for SDL2 and its extensions using PkgConfig
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_IMAGE SDL2_image)
    pkg_check_modules(SDL2_TTF SDL2_ttf)
    pkg_check_modules(SDL2_GFX SDL2_gfx)

    # Add include directories found by PkgConfig
    target_include_directories(VirtualKeyboard PRIVATE
            ${SDL2_INCLUDE_DIRS}
            ${SDL2_IMAGE_INCLUDE_DIRS}
            ${SDL2_TTF_INCLUDE_DIRS}
            ${SDL2_GFX_INCLUDE_DIRS}
    )

    # Link against the libraries found by PkgConfig
    target_link_libraries(VirtualKeyboard PRIVATE
            ${SDL2_GFX_LIBRARIES}
            ${SDL2_IMAGE_LIBRARIES}
            ${SDL2_TTF_LIBRARIES}
            ${SDL2_LIBRARIES}
    )
endif()
