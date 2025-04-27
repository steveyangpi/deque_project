cmake_minimum_required(VERSION 3.22)
project(deque_project)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(dequeLib
        include/deque.h
        src/deque.cpp
)

target_include_directories(dequeLib PUBLIC include)

add_executable(deque src/main.cpp)
target_link_libraries(deque dequeLib)