cmake_minimum_required(VERSION 3.27)
project(P2MemoryManager)

set(CMAKE_CXX_STANDARD 17)

add_executable(P2MemoryManager MemoryManager.cpp
        CommandLineTest.cpp
)
