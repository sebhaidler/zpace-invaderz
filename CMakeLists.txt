# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.

cmake_minimum_required(VERSION 3.14) # FetchContent_MakeAvailable requires 3.14

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Zpace Invaderz")

# Add dependencies
include(FetchContent)

# add dependencies here
FetchContent_Declare(
    SDL2
    URL https://libsdl.org/release/SDL2-2.0.20.tar.gz
)

set(SDL2MIXER_INSTALL OFF)
set(SDL2TTF_INSTALL OFF)
set(BUILD_SHARED_LIBS FALSE)

FetchContent_Declare(
    SDL2_TTF
    GIT_REPOSITORY  https://github.com/libsdl-org/SDL_TTF
    GIT_TAG         release-2.24.0
    GIT_PROGRESS    TRUE
)
FetchContent_Declare(
    SDL2_MIXER
    GIT_REPOSITORY  https://github.com/libsdl-org/SDL_mixer
    GIT_TAG         release-2.6.3
    GIT_PROGRESS    TRUE
)

FetchContent_MakeAvailable(SDL2 SDL2_TTF SDL2_MIXER)

set(SDL_LIBRARIES ${SDL_LIBRARIES} SDL2main SDL2-static SDL2_ttf SDL2_mixer)

FetchContent_Declare(
    EASYS
    GIT_REPOSITORY  https://github.com/raphaelmayer/easys
    GIT_TAG         v1.0.0 # or any version you prefer
    GIT_PROGRESS    TRUE
)
FetchContent_MakeAvailable(EASYS)

# Define the source assets directory
set(ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")

# Define the destination directory (same as the executable's directory)
set(ASSETS_DEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/assets")

# Create the destination assets directory
file(MAKE_DIRECTORY "${ASSETS_DEST_DIR}")

# Copy all assets (e.g., MP3 files) to the build directory
file(COPY "${ASSETS_SOURCE_DIR}/sixtyfour_convergence_regular.ttf" DESTINATION "${ASSETS_DEST_DIR}")



# Include sub-projects.
add_subdirectory ("src")
