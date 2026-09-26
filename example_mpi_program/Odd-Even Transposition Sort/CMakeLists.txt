cmake_minimum_required(VERSION 3.20)
project(overload CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MPI REQUIRED)

add_executable(main main.cpp new_mpi.h trace_collector.h)
target_link_libraries(main MPI::MPI_CXX)