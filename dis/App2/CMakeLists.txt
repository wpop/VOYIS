cmake_minimum_required(VERSION 3.10) # Specify the minimum required CMake version
project(FeatureExtractor CXX)        # Define your project name and language

# Find the OpenCV package
find_package(OpenCV REQUIRED)

# Include OpenCV's header directories
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(/usr/local/include/opencv4)
include_directories(/usr/include/opencv4)

# Add your executable target
add_executable(feature app2.cpp FeatureExtractor.cpp) # Replace main.cpp with your source file(s)

# Link your executable with OpenCV's libraries
target_link_libraries(feature ${OpenCV_LIBS})