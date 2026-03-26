#pragma once
#include "new_mpi.h"

#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstring>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (size < 2) {
        if (rank == 0) {
            std::cerr << "This program requires at least 2 processes" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }
    
    const int DATA_SIZE = 10;
    const int ROOT = 0;
    
    // ==================== 1. MPI_Bcast ====================
    if (rank == 0) {
        std::cout << "\n=== 1. MPI_Bcast ===" << std::endl;
    }
    
    int bcast_data = (rank == 0) ? 12345 : 0;
    MPI_Bcast(&bcast_data, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    
    std::cout << "Process " << rank << " received from Bcast: " << bcast_data << std::endl;
    MPI_Barrier(MPI_COMM_WORLD);
    
    // ==================== 2. MPI_Reduce ====================
    if (rank == 0) {
        std::cout << "\n=== 2. MPI_Reduce ===" << std::endl;
    }
    
    int local_value = rank + 1;
    int global_sum = 0;
    MPI_Reduce(&local_value, &global_sum, 1, MPI_INT, MPI_SUM, ROOT, MPI_COMM_WORLD);
    
    if (rank == ROOT) {
        std::cout << "Reduce sum: " << global_sum << " (expected: " << size * (size + 1) / 2 << ")" << std::endl;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    
    // ==================== 3. MPI_Gather ====================
    if (rank == 0) {
        std::cout << "\n=== 3. MPI_Gather ===" << std::endl;
    }
    
    int send_val = rank * 10;
    std::vector<int> gather_recv(size);
    MPI_Gather(&send_val, 1, MPI_INT, gather_recv.data(), 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    
    if (rank == ROOT) {
        std::cout << "Gather results: ";
        for (int i = 0; i < size; i++) {
            std::cout << gather_recv[i] << " ";
        }
        std::cout << std::endl;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    
    // ==================== 4. MPI_Scatter ====================
    if (rank == 0) {
        std::cout << "\n=== 4. MPI_Scatter ===" << std::endl;
    }
    
    std::vector<int> scatter_send(size);
    int scatter_recv;
    
    if (rank == ROOT) {
        for (int i = 0; i < size; i++) {
            scatter_send[i] = i * 100;
        }
    }
    
    MPI_Scatter(scatter_send.data(), 1, MPI_INT, &scatter_recv, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    std::cout << "Process " << rank << " received from Scatter: " << scatter_recv << std::endl;
    MPI_Barrier(MPI_COMM_WORLD);
    
    // ==================== 5. MPI_Gatherv ====================
    if (rank == 0) {
        std::cout << "\n=== 5. MPI_Gatherv ===" << std::endl;
    }
    
    int sendcount = rank + 1;
    std::vector<int> sendbuf(sendcount);
    for (int i = 0; i < sendcount; i++) {
        sendbuf[i] = rank * 100 + i;
    }
    
    std::vector<int> recvcounts(size);
    std::vector<int> displs(size);
    std::vector<int> gatherv_recv;
    
    if (rank == ROOT) {
        for (int i = 0; i < size; i++) {
            recvcounts[i] = i + 1;
        }
        displs[0] = 0;
        for (int i = 1; i < size; i++) {
            displs[i] = displs[i-1] + recvcounts[i-1];
        }
        int total = displs[size-1] + recvcounts[size-1];
        gatherv_recv.resize(total);
    }
    
    MPI_Gatherv(sendbuf.data(), sendcount, MPI_INT,
                gatherv_recv.data(), recvcounts.data(), displs.data(), MPI_INT,
                ROOT, MPI_COMM_WORLD);
    
    if (rank == ROOT) {
        std::cout << "Gatherv results:" << std::endl;
        for (int i = 0; i < size; i++) {
            std::cout << "  From process " << i << " (size=" << recvcounts[i] << "): ";
            for (int j = 0; j < recvcounts[i]; j++) {
                std::cout << gatherv_recv[displs[i] + j] << " ";
            }
            std::cout << std::endl;
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    
    // ==================== 6. MPI_Scatterv ====================
    if (rank == 0) {
        std::cout << "\n=== 6. MPI_Scatterv ===" << std::endl;
    }
    
    std::vector<int> scatterv_sendbuf;
    std::vector<int> sendcounts(size);
    std::vector<int> scatterv_displs(size);
    
    if (rank == ROOT) {
        for (int i = 0; i < size; i++) {
            sendcounts[i] = i + 1;
        }
        scatterv_displs[0] = 0;
        for (int i = 1; i < size; i++) {
            scatterv_displs[i] = scatterv_displs[i-1] + sendcounts[i-1];
        }
        int total = scatterv_displs[size-1] + sendcounts[size-1];
        scatterv_sendbuf.resize(total);
        for (int i = 0; i < total; i++) {
            scatterv_sendbuf[i] = i * 100;
        }
    }
    
    int recvcount_v = rank + 1;
    std::vector<int> scatterv_recvbuf(recvcount_v);
    
    MPI_Scatterv(scatterv_sendbuf.data(), sendcounts.data(), scatterv_displs.data(), MPI_INT,
                 scatterv_recvbuf.data(), recvcount_v, MPI_INT,
                 ROOT, MPI_COMM_WORLD);
    
    std::cout << "Process " << rank << " received from Scatterv (" << recvcount_v << " elements): ";
    for (int val : scatterv_recvbuf) std::cout << val << " ";
    std::cout << std::endl;
    MPI_Barrier(MPI_COMM_WORLD);
    
    // ==================== 7. MPI_Ibcast (неблокирующий) ====================
    if (rank == 0) {
        std::cout << "\n=== 7. MPI_Ibcast ===" << std::endl;
    }
    
    int ibcast_data = (rank == 0) ? 99999 : 0;
    MPI_Request ibcast_request;
    MPI_Ibcast(&ibcast_data, 1, MPI_INT, ROOT, MPI_COMM_WORLD, &ibcast_request);
    MPI_Wait(&ibcast_request, MPI_STATUS_IGNORE);
    
    std::cout << "Process " << rank << " received from Ibcast: " << ibcast_data << std::endl;
    MPI_Barrier(MPI_COMM_WORLD);
    
    // ==================== 8. MPI_Ireduce ====================
    if (rank == 0) {
        std::cout << "\n=== 8. MPI_Ireduce ===" << std::endl;
    }
    
    int ilocal_val = rank * 5;
    int iglobal_sum = 0;
    MPI_Request ireduce_request;
    MPI_Ireduce(&ilocal_val, &iglobal_sum, 1, MPI_INT, MPI_SUM, ROOT, MPI_COMM_WORLD, &ireduce_request);
    MPI_Wait(&ireduce_request, MPI_STATUS_IGNORE);
    
    if (rank == ROOT) {
        std::cout << "Ireduce sum: " << iglobal_sum << std::endl;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    
    // ==================== 9. MPI_Igather ====================
    if (rank == 0) {
        std::cout << "\n=== 9. MPI_Igather ===" << std::endl;
    }
    
    int igather_send = rank * 100;
    std::vector<int> igather_recv(size);
    MPI_Request igather_request;
    MPI_Igather(&igather_send, 1, MPI_INT, igather_recv.data(), 1, MPI_INT, ROOT, MPI_COMM_WORLD, &igather_request);
    MPI_Wait(&igather_request, MPI_STATUS_IGNORE);
    
    if (rank == ROOT) {
        std::cout << "Igather results: ";
        for (int v : igather_recv) std::cout << v << " ";
        std::cout << std::endl;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    
    // ==================== 10. MPI_Iscatter ====================
    if (rank == 0) {
        std::cout << "\n=== 10. MPI_Iscatter ===" << std::endl;
    }
    
    std::vector<int> iscatter_send(size);
    int iscatter_recv;
    MPI_Request iscatter_request;
    
    if (rank == ROOT) {
        for (int i = 0; i < size; i++) {
            iscatter_send[i] = i * 500;
        }
    }
    
    MPI_Iscatter(iscatter_send.data(), 1, MPI_INT, &iscatter_recv, 1, MPI_INT, ROOT, MPI_COMM_WORLD, &iscatter_request);
    MPI_Wait(&iscatter_request, MPI_STATUS_IGNORE);
    
    std::cout << "Process " << rank << " received from Iscatter: " << iscatter_recv << std::endl;
    MPI_Barrier(MPI_COMM_WORLD);
    
    // ==================== 11. MPI_Igatherv ====================
    if (rank == 0) {
        std::cout << "\n=== 11. MPI_Igatherv ===" << std::endl;
    }
    
    int isendcount = rank + 1;
    std::vector<int> isendbuf(isendcount);
    for (int i = 0; i < isendcount; i++) {
        isendbuf[i] = rank * 1000 + i;
    }
    
    std::vector<int> irecvcounts(size);
    std::vector<int> idispls(size);
    std::vector<int> igatherv_recv;
    MPI_Request igatherv_request;
    
    if (rank == ROOT) {
        for (int i = 0; i < size; i++) {
            irecvcounts[i] = i + 1;
        }
        idispls[0] = 0;
        for (int i = 1; i < size; i++) {
            idispls[i] = idispls[i-1] + irecvcounts[i-1];
        }
        int total = idispls[size-1] + irecvcounts[size-1];
        igatherv_recv.resize(total);
    }
    
    MPI_Igatherv(isendbuf.data(), isendcount, MPI_INT,
                 igatherv_recv.data(), irecvcounts.data(), idispls.data(), MPI_INT,
                 ROOT, MPI_COMM_WORLD, &igatherv_request);
    MPI_Wait(&igatherv_request, MPI_STATUS_IGNORE);
    
    if (rank == ROOT) {
        std::cout << "Igatherv results:" << std::endl;
        for (int i = 0; i < size; i++) {
            std::cout << "  From process " << i << " (size=" << irecvcounts[i] << "): ";
            for (int j = 0; j < irecvcounts[i]; j++) {
                std::cout << igatherv_recv[idispls[i] + j] << " ";
            }
            std::cout << std::endl;
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    
    // ==================== 12. MPI_Iscatterv ====================
    if (rank == 0) {
        std::cout << "\n=== 12. MPI_Iscatterv ===" << std::endl;
    }
    
    std::vector<int> iscatterv_sendbuf;
    std::vector<int> isendcounts(size);
    std::vector<int> iscatterv_displs(size);
    MPI_Request iscatterv_request;
    
    if (rank == ROOT) {
        for (int i = 0; i < size; i++) {
            isendcounts[i] = i + 1;
        }
        iscatterv_displs[0] = 0;
        for (int i = 1; i < size; i++) {
            iscatterv_displs[i] = iscatterv_displs[i-1] + isendcounts[i-1];
        }
        int total = iscatterv_displs[size-1] + isendcounts[size-1];
        iscatterv_sendbuf.resize(total);
        for (int i = 0; i < total; i++) {
            iscatterv_sendbuf[i] = i * 50;
        }
    }
    
    int irecvcount_v = rank + 1;
    std::vector<int> iscatterv_recvbuf(irecvcount_v);
    
    MPI_Iscatterv(iscatterv_sendbuf.data(), isendcounts.data(), iscatterv_displs.data(), MPI_INT,
                  iscatterv_recvbuf.data(), irecvcount_v, MPI_INT,
                  ROOT, MPI_COMM_WORLD, &iscatterv_request);
    MPI_Wait(&iscatterv_request, MPI_STATUS_IGNORE);
    
    std::cout << "Process " << rank << " received from Iscatterv (" << irecvcount_v << " elements): ";
    for (int val : iscatterv_recvbuf) std::cout << val << " ";
    std::cout << std::endl;
    MPI_Barrier(MPI_COMM_WORLD);
    
    // ==================== ИТОГ ====================
    if (rank == 0) {
        std::cout << "\n=== All collective operations completed successfully! ===" << std::endl;
    }
    
    MPI_Finalize();
    return 0;
}