# add_library(SharedMemory SHARED SharedMemory.cpp)
# target_include_directories(SharedMemory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
cmake_minimum_required(VERSION 3.10) # Specify the minimum required CMake version
project(SharedMemory CXX)         # Define your project name and language

# Find the OpenCV package
# REQUIRED ensures that CMake will fail if OpenCV is not found
find_package(OpenCV REQUIRED)

# Include OpenCV's header directories
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(/usr/local/include/opencv4)
include_directories(/usr/include/opencv4)

# Add your executable target
add_executable(shared_lib lib.cpp SharedMemory.cpp) # Replace main.cpp with your source file(s)

# Link your executable with OpenCV's libraries
target_link_libraries(shared_lib ${OpenCV_LIBS})
