cmake_minimum_required(VERSION 3.10)
project(MPISum LANGUAGES CXX)

# Явное указание компиляторов с Score-P
set(CMAKE_C_COMPILER "scorep")
set(CMAKE_C_COMPILER_ARG1 "mpicc")
set(CMAKE_CXX_COMPILER "scorep") 
set(CMAKE_CXX_COMPILER_ARG1 "mpicxx")

find_package(MPI REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lm")

add_executable(simple_mpi main.cpp)
target_link_libraries(simple_mpi MPI::MPI_CXX)
