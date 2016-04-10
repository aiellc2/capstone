cmake_minimum_required(VERSION 2.8)
project( cap )
find_package( OpenCV REQUIRED )
file(GLOB cap_SRC
    "*.hpp"
    "*.cpp"
)
add_executable( cap ${cap_SRC} )
target_link_libraries( cap ${OpenCV_LIBS} )
