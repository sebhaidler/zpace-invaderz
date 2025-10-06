# CMakeList.txt : CMake project for Simple SDL2 setup, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (CMakeTarget "main.cpp" "main.h" "components.hpp" "systems.hpp")

# Link the necessary libraries, including SDL
target_link_libraries(CMakeTarget PRIVATE ${SDL_LIBRARIES})


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CMakeTarget PROPERTY CXX_STANDARD 20)
endif()

target_include_directories(CMakeTarget PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")



# TODO: Add tests and install targets if needed.
target_link_libraries(CMakeTarget PRIVATE easys)

# Function to copy DLLs to the output directory
#function(copy_dll target dll_file)
#    add_custom_command(TARGET ${target} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            "${CMAKE_CURRENT_SOURCE_DIR}/${dll_file}"
 #           $<TARGET_FILE_DIR:${target}>
#    )
#endfunction()

# Copy SDL2_mixer.dll to the output directory
#copy_dll(CMakeTarget "SDL2_mixer.dll")