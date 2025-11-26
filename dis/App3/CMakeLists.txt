cmake_minimum_required(VERSION 3.10) # Specify the minimum required CMake version
project(DataLogger CXX)         	 # Define your project name and language

find_package(OpenCV REQUIRED)
find_package(SQLite3 REQUIRED)

# Include OpenCV's header directories
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(/usr/local/include/opencv4)
include_directories(/usr/include/opencv4)

# Add your executable target
add_executable(loger app3.cpp ImgDBManager.cpp) # Replace main.cpp with your source file(s)

# Link your executable with OpenCV's libraries
target_link_libraries(loger ${OpenCV_LIBS})
