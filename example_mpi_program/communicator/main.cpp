#include "new_mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size % 2 != 0) {
        if (rank == 0) {
            printf("Требуется чётное число процессов\n");
        }
        MPI_Finalize();
        return 1;
    }

    printf("Ранг %d: старт\n", rank);

    // ========== 1. MPI_Comm_split: делим на пары ==========
    MPI_Comm pair_comm;
    int color = rank / 2;
    int key   = rank % 2;

    MPI_Comm_split(MPI_COMM_WORLD, color, key, &pair_comm);

    int pair_rank, pair_size;
    MPI_Comm_rank(pair_comm, &pair_rank);
    MPI_Comm_size(pair_comm, &pair_size);

    int value = rank;
    MPI_Bcast(&value, 1, MPI_INT, 0, pair_comm);

    printf("Ранг %d: split -> группа %d, pair_rank %d/%d, "
           "после Bcast value = %d\n",
           rank, color, pair_rank, pair_size, value);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Comm_free(&pair_comm);

    // ========== 2. MPI_Comm_create: группа из трёх процессов {0, 1, 2} ==========
    MPI_Group world_group2, triple_group;
    MPI_Comm_group(MPI_COMM_WORLD, &world_group2);

    int triple_ranks[3] = {0, 1, 2};
    MPI_Group_incl(world_group2, 3, triple_ranks, &triple_group);

    MPI_Comm triple_comm = MPI_COMM_NULL;
    MPI_Comm_create(MPI_COMM_WORLD, triple_group, &triple_comm);

    if (triple_comm != MPI_COMM_NULL) {
        int t_rank, t_size;
        MPI_Comm_rank(triple_comm, &t_rank);
        MPI_Comm_size(triple_comm, &t_size);

        int t_sum = 0;
        MPI_Allreduce(&rank, &t_sum, 1, MPI_INT, MPI_SUM, triple_comm);

        printf("Ранг %d: create(triple) -> rank %d/%d, sum = %d\n",
               rank, t_rank, t_size, t_sum);

        MPI_Comm_free(&triple_comm);
    } else {
        printf("Ранг %d: не вошёл в triple_comm\n", rank);
    }

    MPI_Group_free(&triple_group);
    MPI_Group_free(&world_group2);

    MPI_Barrier(MPI_COMM_WORLD);

    // ========== 3. MPI_Comm_dup: дубликат MPI_COMM_WORLD ==========
    MPI_Comm dup_comm;
    MPI_Comm_dup(MPI_COMM_WORLD, &dup_comm);

    int dup_rank, dup_size;
    MPI_Comm_rank(dup_comm, &dup_rank);
    MPI_Comm_size(dup_comm, &dup_size);

    int sum = 0;
    MPI_Allreduce(&rank, &sum, 1, MPI_INT, MPI_SUM, dup_comm);

    printf("Ранг %d: dup -> rank %d/%d, Allreduce sum = %d\n",
           rank, dup_rank, dup_size, sum);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Comm_free(&dup_comm);

    // ========== 4. MPI_Comm_create: группа чётных рангов ==========
    MPI_Group world_group, even_group;
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);

    int even_ranks[128];
    int n_even = 0;
    for (int i = 0; i < size; i += 2) {
        even_ranks[n_even++] = i;
    }

    MPI_Group_incl(world_group, n_even, even_ranks, &even_group);

    MPI_Comm even_comm = MPI_COMM_NULL;
    MPI_Comm_create(MPI_COMM_WORLD, even_group, &even_comm);

    if (even_comm != MPI_COMM_NULL) {
        int even_rank, even_size;
        MPI_Comm_rank(even_comm, &even_rank);
        MPI_Comm_size(even_comm, &even_size);

        int total = 0;
        MPI_Allreduce(&rank, &total, 1, MPI_INT, MPI_SUM, even_comm);

        printf("Ранг %d: create(even) -> rank %d/%d, sum = %d\n",
               rank, even_rank, even_size, total);

        MPI_Comm_free(&even_comm);
    } else {
        printf("Ранг %d: не вошёл в even_comm\n", rank);
    }

    MPI_Group_free(&even_group);
    MPI_Group_free(&world_group);

    MPI_Barrier(MPI_COMM_WORLD);

    // ========== Финализация ==========
    MPI_Finalize();
    return 0;
}