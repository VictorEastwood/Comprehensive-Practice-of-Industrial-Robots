# cmake_minimum_required(VERSION 3.28)

cmake_minimum_required(VERSION 3.16.3)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project(Project)

# OpenCV Library
find_package(OpenCV REQUIRED)
link_directories(${OpenCV_LIBRARIES})
include_directories(${OpenCV_INCLUDE_DIRS})


file(GLOB USER_SOURCES "Application/*.*" 
                        "Lib/*.*"
                        "Driver/*.*"
                        "Algorithm/*.*"

)


include_directories(    Application/
                        Lib/
                        Lib/eigen-3.4.0
                        Driver/
                        Algorithm/
)

add_executable(Project main.cpp
        main.hpp
        ${USER_SOURCES}
)

target_link_libraries(Project ${OpenCV_LIBS}
                              ws2_32
)


