cmake_minimum_required(VERSION 3.16)
project(redis-clone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Release build by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

add_executable(redis-clone
    main.cpp
    Server.cpp
    Connection.cpp
    EventLoop.cpp
    HashTable.cpp
)

target_link_libraries(redis-clone PRIVATE pthread)
