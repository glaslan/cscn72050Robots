cmake_minimum_required(VERSION 3.7)

project(Webpage)

set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(Threads)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(server main.cpp MySocket.cpp MySocket.h PktDef.cpp PktDef.h)
target_link_libraries(server ${Boost_LIBRARIES} Threads::Threads)