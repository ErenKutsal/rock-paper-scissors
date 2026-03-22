cmake_minimum_required(VERSION 3.10)
project(RockPaperScissors)

# Set the C++ standard (C++17 is great for modern features like auto and iterators)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find required system and third-party packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Define the executable name and its source files
# Note: If your main file is not named 'main.cpp', change it here!
add_executable(rps 
    main.cpp 
    ../util/InitShader.cpp
)

# Tell the compiler where to find headers like "InitShader.h"
target_include_directories(rps PRIVATE ../util)

# Link the required libraries to the executable
target_link_libraries(rps PRIVATE 
    OpenGL::GL 
    glfw 
    glm::glm
)