cmake_minimum_required(VERSION 3.15)

project(    CppMultithreadingLearning
            VERSION 1.0
            DESCRIPTION "Development environment for multithreading programming in C++"
            LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Threads REQUIRED)

add_subdirectory(src)
