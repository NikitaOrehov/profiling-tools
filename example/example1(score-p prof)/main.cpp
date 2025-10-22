
#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <chrono>
#include <complex>

// Пользовательские регионы для Score-P
#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>
#endif

class ComplexMPIApp {
private:
    int rank, size;
    std::vector<double> local_data;
    std::vector<double> received_data;
    MPI_Comm custom_comm;
    MPI_Win window;
    double* window_data;

public:
    ComplexMPIApp(int argc, char** argv) {
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        // Создаем кастомный коммуникатор
        int color = rank % 2;
        MPI_Comm_split(MPI_COMM_WORLD, color, rank, &custom_comm);

        // Инициализируем данные
        initialize_data();
        
        // Создаем MPI окно для RMA
        setup_rma_window();
    }

    ~ComplexMPIApp() {
        MPI_Win_free(&window);
        MPI_Comm_free(&custom_comm);
        MPI_Finalize();
    }

private:
    void initialize_data() {
        #ifdef USE_SCOREP
        SCOREP_USER_REGION_DEFINE(init_region)
        SCOREP_USER_REGION_BEGIN(init_region, "initialize_data", SCOREP_USER_REGION_TYPE_COMMON)
        #endif

        int local_size = 10000 + rank * 100;  // Разные размеры для дисбаланса
        local_data.resize(local_size);
        received_data.resize(local_size);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 100.0);

        for (int i = 0; i < local_size; ++i) {
            local_data[i] = dis(gen);
        }

        #ifdef USE_SCOREP
        SCOREP_USER_REGION_END(init_region)
        #endif
    }

    void setup_rma_window() {
        int window_size = 1000;
        window_data = new double[window_size];
        
        for (int i = 0; i < window_size; ++i) {
            window_data[i] = rank * 1000 + i;
        }

        MPI_Win_create(window_data, window_size * sizeof(double), sizeof(double),
                      MPI_INFO_NULL, MPI_COMM_WORLD, &window);
    }

public:
    void run_computation() {
        #ifdef USE_SCOREP
        SCOREP_USER_REGION_DEFINE(comp_region)
        SCOREP_USER_REGION_BEGIN(comp_region, "run_computation", SCOREP_USER_REGION_TYPE_COMMON)
        #endif

        // 1. Тяжелые вычисления
        perform_heavy_computations();
        
        // 2. Коллективные операции
        perform_collective_operations();
        
        // 3. Point-to-point коммуникации
        perform_point_to_point_communications();
        
        // 4. RMA операции
        perform_rma_operations();
        
        // 5. Несбалансированная работа
        perform_imbalanced_work();
        
        // 6. MPI I/O операции
        perform_io_operations();

        #ifdef USE_SCOREP
        SCOREP_USER_REGION_END(comp_region)
        #endif
    }

private:
    void perform_heavy_computations() {
        #ifdef USE_SCOREP
        SCOREP_USER_REGION_DEFINE(heavy_comp_region)
        SCOREP_USER_REGION_BEGIN(heavy_comp_region, "heavy_computations", SCOREP_USER_REGION_TYPE_COMMON)
        #endif

        // Матричные операции
        int matrix_size = 500;
        std::vector<std::vector<double>> matrix(matrix_size, std::vector<double>(matrix_size));
        
        // Заполнение матрицы
        for (int i = 0; i < matrix_size; ++i) {
            for (int j = 0; j < matrix_size; ++j) {
                matrix[i][j] = std::sin(i * 0.1) * std::cos(j * 0.1);
            }
        }

        // Умножение матриц
        std::vector<std::vector<double>> result(matrix_size, std::vector<double>(matrix_size));
        for (int i = 0; i < matrix_size; ++i) {
            for (int j = 0; j < matrix_size; ++j) {
                for (int k = 0; k < matrix_size; ++k) {
                    result[i][j] += matrix[i][k] * matrix[k][j];
                }
            }
        }

        // FFT-подобные вычисления
        std::vector<std::complex<double>> complex_data(1024);
        for (int i = 0; i < 1024; ++i) {
            complex_data[i] = std::complex<double>(
                std::cos(i * 0.01), std::sin(i * 0.01)
            );
        }

        // Имитация FFT
        for (int stage = 0; stage < 10; ++stage) {
            for (int i = 0; i < 512; ++i) {
                std::complex<double> temp = complex_data[i];
                complex_data[i] = complex_data[i + 512];
                complex_data[i + 512] = temp;
            }
        }

        #ifdef USE_SCOREP
        SCOREP_USER_REGION_END(heavy_comp_region)
        #endif
    }

    void perform_collective_operations() {
        #ifdef USE_SCOREP
        SCOREP_USER_REGION_DEFINE(coll_region)
        SCOREP_USER_REGION_BEGIN(coll_region, "collective_operations", SCOREP_USER_REGION_TYPE_COMMON)
        #endif

        // Различные коллективные операции
        
        // 1. Broadcast
        double broadcast_data = rank * 3.14;
        MPI_Bcast(&broadcast_data, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // 2. Reduce
        double local_sum = 0.0;
        for (double val : local_data) local_sum += val;
        
        double global_sum;
        MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

        // 3. Allreduce
        double allreduce_result;
        MPI_Allreduce(&local_sum, &allreduce_result, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

        // 4. Scatter
        std::vector<double> scatter_send;
        if (rank == 0) {
            scatter_send.resize(size * 100);
            std::fill(scatter_send.begin(), scatter_send.end(), 1.0);
        }
        std::vector<double> scatter_recv(100);
        MPI_Scatter(scatter_send.data(), 100, MPI_DOUBLE,
                   scatter_recv.data(), 100, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        // 5. Gather
        std::vector<double> gather_recv;
        if (rank == 0) gather_recv.resize(size * 100);
        MPI_Gather(scatter_recv.data(), 100, MPI_DOUBLE,
                  gather_recv.data(), 100, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // 6. Alltoall
        std::vector<double> alltoall_send(size, rank * 1.0);
        std::vector<double> alltoall_recv(size);
        MPI_Alltoall(alltoall_send.data(), 1, MPI_DOUBLE,
                    alltoall_recv.data(), 1, MPI_DOUBLE, MPI_COMM_WORLD);

        // 7. Barrier
        MPI_Barrier(MPI_COMM_WORLD);

        // Коллективные операции в кастомном коммуникаторе
        double custom_sum;
        MPI_Allreduce(&local_sum, &custom_sum, 1, MPI_DOUBLE, MPI_SUM, custom_comm);

        #ifdef USE_SCOREP
        SCOREP_USER_REGION_END(coll_region)
        #endif
    }

    void perform_point_to_point_communications() {
        #ifdef USE_SCOREP
        SCOREP_USER_REGION_DEFINE(p2p_region)
        SCOREP_USER_REGION_BEGIN(p2p_region, "point_to_point", SCOREP_USER_REGION_TYPE_COMMON)
        #endif

        // Различные P2P операции
        
        int next_rank = (rank + 1) % size;
        int prev_rank = (rank - 1 + size) % size;

        // 1. Блокирующие send/recv
        double send_data = rank * 10.0;
        double recv_data;
        MPI_Send(&send_data, 1, MPI_DOUBLE, next_rank, 0, MPI_COMM_WORLD);
        MPI_Recv(&recv_data, 1, MPI_DOUBLE, prev_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // 2. Неблокирующие isend/irecv
        MPI_Request requests[4];
        double nonblock_data[2] = {rank * 20.0, rank * 30.0};
        double nonblock_recv[2];

        MPI_Isend(&nonblock_data[0], 1, MPI_DOUBLE, next_rank, 1, MPI_COMM_WORLD, &requests[0]);
        MPI_Isend(&nonblock_data[1], 1, MPI_DOUBLE, prev_rank, 2, MPI_COMM_WORLD, &requests[1]);
        MPI_Irecv(&nonblock_recv[0], 1, MPI_DOUBLE, prev_rank, 1, MPI_COMM_WORLD, &requests[2]);
        MPI_Irecv(&nonblock_recv[1], 1, MPI_DOUBLE, next_rank, 2, MPI_COMM_WORLD, &requests[3]);

        // Вычисления во время коммуникаций
        perform_light_computations();

        MPI_Waitall(4, requests, MPI_STATUSES_IGNORE);

        // 3. Sendrecv для избежания deadlock
        double sendrecv_send = rank * 40.0;
        double sendrecv_recv;
        MPI_Sendrecv(&sendrecv_send, 1, MPI_DOUBLE, next_rank, 3,
                    &sendrecv_recv, 1, MPI_DOUBLE, prev_rank, 3,
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        #ifdef USE_SCOREP
        SCOREP_USER_REGION_END(p2p_region)
        #endif
    }

    void perform_light_computations() {
        // Легкие вычисления во время неблокирующих операций
        double sum = 0.0;
        for (int i = 0; i < 1000; ++i) {
            sum += std::sqrt(i) * std::log(i + 1);
        }
    }

    void perform_rma_operations() {
        #ifdef USE_SCOREP
        SCOREP_USER_REGION_DEFINE(rma_region)
        SCOREP_USER_REGION_BEGIN(rma_region, "rma_operations", SCOREP_USER_REGION_TYPE_COMMON)
        #endif

        // Односторонние коммуникации (RMA)
        
        int target_rank = (rank + 1) % size;
        double put_data = rank * 100.0;
        double get_data;

        // Синхронизация для RMA
        MPI_Win_fence(0, window);

        // 1. Put операция
        MPI_Put(&put_data, 1, MPI_DOUBLE, target_rank, 
                rank * 10, 1, MPI_DOUBLE, window);

        // 2. Get операция  
        MPI_Get(&get_data, 1, MPI_DOUBLE, target_rank,
                target_rank * 10, 1, MPI_DOUBLE, window);

        MPI_Win_fence(0, window);

        // 3. Accumulate
        double acc_data = 5.0;
        MPI_Accumulate(&acc_data, 1, MPI_DOUBLE, target_rank,
                      rank * 20, 1, MPI_DOUBLE, MPI_SUM, window);

        MPI_Win_fence(0, window);

        #ifdef USE_SCOREP
        SCOREP_USER_REGION_END(rma_region)
        #endif
    }

    void perform_imbalanced_work() {
        #ifdef USE_SCOREP
        SCOREP_USER_REGION_DEFINE(imbalance_region)
        SCOREP_USER_REGION_BEGIN(imbalance_region, "imbalanced_work", SCOREP_USER_REGION_TYPE_COMMON)
        #endif

        // Создаем преднамеренный дисбаланс нагрузки
        int work_load = 1000000 * (rank + 1);  // Процесс 0 делает меньше, процесс N-1 делает больше

        double result = 0.0;
        for (int i = 0; i < work_load; ++i) {
            result += std::sin(i * 0.0001) * std::cos(i * 0.0001);
        }

        #ifdef USE_SCOREP
        SCOREP_USER_REGION_END(imbalance_region)
        #endif

        // Синхронизация после несбалансированной работы
        MPI_Barrier(MPI_COMM_WORLD);
    }

    void perform_io_operations() {
        #ifdef USE_SCOREP
        SCOREP_USER_REGION_DEFINE(io_region)
        SCOREP_USER_REGION_BEGIN(io_region, "io_operations", SCOREP_USER_REGION_TYPE_COMMON)
        #endif

        // MPI I/O операции
        
        MPI_File file;
        char filename[100];
        sprintf(filename, "output_%d.bin", rank);

        // 1. Создание и запись файла
        MPI_File_open(MPI_COMM_SELF, filename, 
                     MPI_MODE_CREATE | MPI_MODE_WRONLY, 
                     MPI_INFO_NULL, &file);

        std::vector<double> io_data(1000);
        for (int i = 0; i < 1000; ++i) {
            io_data[i] = rank * 1000 + i;
        }

        MPI_File_write(file, io_data.data(), 1000, MPI_DOUBLE, MPI_STATUS_IGNORE);
        MPI_File_close(&file);

        // 2. Коллективный I/O
        if (size > 1) {
            MPI_File collective_file;
            MPI_File_open(MPI_COMM_WORLD, "collective_output.bin",
                         MPI_MODE_CREATE | MPI_MODE_WRONLY,
                         MPI_INFO_NULL, &collective_file);

            MPI_File_write_all(collective_file, io_data.data(), 1000, MPI_DOUBLE, MPI_STATUS_IGNORE);
            MPI_File_close(&collective_file);
        }

        #ifdef USE_SCOREP
        SCOREP_USER_REGION_END(io_region)
        #endif
    }
};

int main(int argc, char** argv) {
    #ifdef USE_SCOREP
    SCOREP_USER_REGION_DEFINE(main_region)
    SCOREP_USER_REGION_BEGIN(main_region, "main", SCOREP_USER_REGION_TYPE_COMMON)
    #endif

    ComplexMPIApp app(argc, argv);
    
    // Запускаем вычисления несколько раз для сбора статистики
    for (int iteration = 0; iteration < 3; ++iteration) {
        app.run_computation();
    }

    #ifdef USE_SCOREP
    SCOREP_USER_REGION_END(main_region)
    #endif

    return 0;
}




























// #include <iostream>
// #include <stdlib.h>
// #include <mpi.h>
// #include <random>

// int main(int argc, char** argv) {
//     if (argv[1] == nullptr){
//         printf("you forgot arg\n");
//         return 0;
//     }

//     MPI_Init(&argc, &argv);

//     int size, rank;
//     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//     MPI_Comm_size(MPI_COMM_WORLD, &size);

//     int real_size = std::stoi(argv[1]);
//     int array_size = std::ceil(static_cast<double>(real_size) / 4) * 4;
//     int local_array_size = array_size / 4;

//     int* local_array_a = new int[local_array_size];
//     int* local_array_b = new int[local_array_size];

//     int* local_result = nullptr;
//     int* array_a = nullptr;
//     int* array_b = nullptr;

//     if (rank == 0){
//         array_a = new int[array_size];
//         array_b = new int[array_size];
//         for (int i = 0; i < array_size; i++){
//             array_a[i] = rand() % 150;
//             array_b[i] = rand() % 150;
//         }
//         std::cout << "a: ";
//         for (int i = 0; i < real_size; i++){
//             std::cout << array_a[i] << " ";
//         }
//         std::cout << "\n";
//         std::cout << "b: ";
//         for (int i = 0; i < real_size; i++){
//             std::cout << array_b[i] << " ";
//         }
//         std::cout << "\n";
//     }

//     MPI_Scatter(array_a, local_array_size, MPI_INT, local_array_a, local_array_size, MPI_INT, 0, MPI_COMM_WORLD);
//     MPI_Scatter(array_b, local_array_size, MPI_INT, local_array_b, local_array_size, MPI_INT, 0, MPI_COMM_WORLD);

//     local_result = new int[local_array_size];
//     for (int i = 0; i < local_array_size; i++){
//         local_result[i] = local_array_a[i] + local_array_b[i];
//     }

//     int* result = nullptr;
//     if (rank == 0){
//         result = new int[array_size];
//     }
//     MPI_Gather(local_result, local_array_size, MPI_INT, result, local_array_size, MPI_INT, 0, MPI_COMM_WORLD);

//     if (rank == 0){
//         std::cout << "result: ";
//         for (int i = 0; i < real_size; i++){
//             std::cout << result[i] << " ";
//         }
//         std::cout << "\n";
//     }


//     MPI_Finalize();
// }