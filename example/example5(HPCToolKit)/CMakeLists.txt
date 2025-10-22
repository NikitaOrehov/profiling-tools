cmake_minimum_required(VERSION 3.10)
project(ProfilingExamples LANGUAGES CXX)

# Используем стандартные MPI компиляторы
set(CMAKE_C_COMPILER mpicc)
set(CMAKE_CXX_COMPILER mpicxx)

find_package(MPI REQUIRED)

# Обязательные флаги для HPCToolkit
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -g -fno-omit-frame-pointer")

add_executable(simple_mpi main.cpp)
target_link_libraries(simple_mpi MPI::MPI_CXX)

message(STATUS "Build configuration:")
message(STATUS "  C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "  C compiler: ${CMAKE_C_COMPILER}")