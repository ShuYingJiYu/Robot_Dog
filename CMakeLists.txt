cmake_minimum_required(VERSION 3.25)
project(tracker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#set(CMAKE_PREFIX_PATH "/home/sakurapuare/Qt/5.15.2/gcc_64/")

find_package(Qt5 COMPONENTS
        Core
        Network
        Widgets
        Gui
        REQUIRED)
find_package(OpenCV REQUIRED)
find_package(lcm REQUIRED)

# glob all the source files
file(GLOB_RECURSE SRC_FILES src/*.cpp)

include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/**)

add_executable(tracker src/main.cpp
)

target_link_libraries(tracker
        Qt5::Core
        Qt5::Network
        Qt5::Widgets
        Qt5::Gui
        lcm
        ${OpenCV_LIBS}
)
add_executable(test src/test.cpp

)
target_link_libraries(test
        Qt5::Core
        Qt5::Network
        #        Qt5::Widgets
        #        Qt5::Gui
        #        lcm
        ${OpenCV_LIBS}
)