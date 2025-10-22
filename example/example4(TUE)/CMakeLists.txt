cmake_minimum_required(VERSION 3.12)
project(MyMPIApp LANGUAGES CXX)

# Всегда используем стандартный компилятор
set(CMAKE_CXX_COMPILER mpicxx)

# Поиск MPI
find_package(MPI REQUIRED)

# Добавляем исполняемый файл
add_executable(mpi_app main.cpp)

# Подключаем MPI
target_link_libraries(mpi_app PUBLIC MPI::MPI_CXX)

# Флаги компиляции
target_compile_features(mpi_app PUBLIC cxx_std_17)
target_compile_options(mpi_app PRIVATE 
    -g 
    -O0
    -Wall
)

set_target_properties(mpi_app PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)