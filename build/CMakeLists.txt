cmake_minimum_required(VERSION 3.4.1)

add_library(py2jib SHARED ../c_wrapper/py2jib.cpp)

find_library(log-lib log)

target_link_libraries(py2jib
    ${log-lib}
    jnigraphics)
