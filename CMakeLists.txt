cmake_minimum_required(VERSION 3.10)
project(py2jib)

add_library(py2jib SHARED c_wrapper/py2jib.cpp)

find_library(log-lib log)

target_link_libraries(py2jib
    ${log-lib}
    jnigraphics)
