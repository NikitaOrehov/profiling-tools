#pragma once
#include "new_mpi.h"

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv) {
    int rank, size;
    int token;
    MPI_Status status;
    MPI_Request request;
    
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (size < 2) {
        printf("This program requires at least 2 processes\n");
        MPI_Finalize();
        return 1;
    }
    
    // Determine neighbors in the ring
    int next = (rank + 1) % size;
    int prev = (rank - 1 + size) % size;
    
    // First part: Synchronous communication demonstration
    if (rank == 0) {
        // Process 0 initiates the token
        token = 42;
        printf("Process %d: Sending token %d to process %d\n", rank, token, next);
        
        // Use synchronous send to ensure completion
        MPI_Ssend(&token, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
        
        // Receive token from the last process
        MPI_Recv(&token, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, &status);
        printf("Process %d: Received final token %d from process %d\n", 
               rank, token, prev);
        
    } else {
        // Other processes receive token
        MPI_Recv(&token, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, &status);
        printf("Process %d: Received token %d from process %d\n", 
               rank, token, prev);
        
        // Modify token
        token += rank;
        
        // Send to next process
        printf("Process %d: Sending token %d to process %d\n", rank, token, next);
        MPI_Ssend(&token, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
    }
    
    // Barrier to ensure first part completes
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Second part: Non-blocking communication with ready send
    // IMPORTANT: For ready send to work, the receive MUST be posted first
    if (rank == 0) {
    int recv_data[10];
    MPI_Request recv_request;
    
    printf("\nProcess %d: Starting non-blocking operations\n", rank);
    
    // First, let process 1 know we're ready by sending a small message
    int ready_signal = 1;
    MPI_Send(&ready_signal, 1, MPI_INT, 1, 10, MPI_COMM_WORLD);
    
    // Post non-blocking receive
    MPI_Irecv(recv_data, 10, MPI_INT, 1, 1, MPI_COMM_WORLD, &recv_request);
    
    printf("Process %d: Non-blocking receive posted, can do other work here\n", rank);
    
    // Можно делать другую работу, пока идёт приём
    
    // Wait for receive to complete
    MPI_Wait(&recv_request, &status);
    
    printf("Process %d: Received %d integers from process %d\n", 
           rank, 10, status.MPI_SOURCE);
    printf("Process %d: First few values: %d, %d, %d\n", 
           rank, recv_data[0], recv_data[1], recv_data[2]);
           
} else if (rank == 1) {
    int data[10];
    int ready_signal;
    MPI_Request send_request;
    
    // Prepare data
    for (int i = 0; i < 10; i++) {
        data[i] = i * rank;
    }
    
    // Wait for signal from process 0 that it's ready to receive
    MPI_Recv(&ready_signal, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, &status);
    
    // Now process 0's receive is definitely posted, so we can use non-blocking ready send
    printf("Process %d: Using non-blocking ready send to process 0\n", rank);
    MPI_Irsend(data, 10, MPI_INT, 0, 1, MPI_COMM_WORLD, &send_request);
    
    printf("Process %d: Non-blocking ready send initiated, can do other work\n", rank);
    
    // Можно делать другую работу, пока идёт отправка
    
    // Wait for send to complete
    MPI_Wait(&send_request, &status);
    printf("Process %d: Non-blocking ready send completed\n", rank);
}
    
    // Barrier after second part
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Third part: Buffered and immediate communications
    if (rank == 0) {
        printf("\nProcess %d: Demonstrating buffered send\n", rank);
        
        // Create buffer for buffered send
        int bufsize = MPI_BSEND_OVERHEAD + 10 * sizeof(int);
        char* buffer = (char*)malloc(bufsize);
        MPI_Buffer_attach(buffer, bufsize);
        
        int buffered_data[10];
        for (int i = 0; i < 10; i++) {
            buffered_data[i] = 100 + i;
        }
        
        // First, let process 1 know we're about to send
        int ready_to_send = 1;
        MPI_Send(&ready_to_send, 1, MPI_INT, 1, 11, MPI_COMM_WORLD);
        
        // Buffered send - will complete immediately if buffer space available
        MPI_Bsend(buffered_data, 10, MPI_INT, 1, 3, MPI_COMM_WORLD);
        printf("Process %d: Buffered send completed immediately\n", rank);
        
        // Detach buffer (will wait for pending communications)
        MPI_Buffer_detach(&buffer, &bufsize);
        free(buffer);
        
    } else if (rank == 1) {
        int received_data[10];
        int ready_signal;
        
        // Wait for signal from process 0
        MPI_Recv(&ready_signal, 1, MPI_INT, 0, 11, MPI_COMM_WORLD, &status);
        
        // Now receive the buffered data
        MPI_Recv(received_data, 10, MPI_INT, 0, 3, MPI_COMM_WORLD, &status);
        printf("Process %d: Received buffered data: ", rank);
        for (int i = 0; i < 5; i++) {
            printf("%d ", received_data[i]);
        }
        printf("...\n");
    }
    
    // Barrier after third part
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Fourth part: Immediate sends with different modes
    if (rank == 2 && size > 2) {
        printf("\nProcess %d: Demonstrating immediate sends\n", rank);
        
        int send_data[5] = {1, 2, 3, 4, 5};
        int recv_data[5];
        MPI_Request req1, req2;
        int ready_signal;
        
        // First, ensure process 0 has posted its receives
        // Send a signal to process 0 to post receives
        int post_receives = 1;
        MPI_Ssend(&post_receives, 1, MPI_INT, 0, 12, MPI_COMM_WORLD);
        
        // Wait for acknowledgment that receives are posted
        MPI_Recv(&ready_signal, 1, MPI_INT, 0, 13, MPI_COMM_WORLD, &status);
        
        // Now we can safely use immediate sends
        // Immediate standard send
        MPI_Isend(send_data, 5, MPI_INT, 0, 4, MPI_COMM_WORLD, &req1);
        
        // Immediate synchronous send
        MPI_Issend(send_data, 5, MPI_INT, 0, 5, MPI_COMM_WORLD, &req2);
        
        // Post receive for the response from process 0
        MPI_Irecv(recv_data, 5, MPI_INT, 0, 6, MPI_COMM_WORLD, &req1);
        
        // Wait for sends to complete
        MPI_Wait(&req1, MPI_STATUS_IGNORE);
        MPI_Wait(&req2, MPI_STATUS_IGNORE);
        
        printf("Process %d: Both immediate sends completed\n", rank);
        
        // Wait for receive to complete
        MPI_Wait(&req1, &status);
        printf("Process %d: Received response: ", rank);
        for (int i = 0; i < 5; i++) {
            printf("%d ", recv_data[i]);
        }
        printf("\n");
        
    } else if (rank == 0 && size > 2) {
        int recv1[5], recv2[5], send_data[5] = {10, 20, 30, 40, 50};
        MPI_Request req3, req4;
        int signal;
        
        // Wait for process 2 to request communication
        MPI_Recv(&signal, 1, MPI_INT, 2, 12, MPI_COMM_WORLD, &status);
        
        // Post receives BEFORE process 2 sends
        MPI_Irecv(recv1, 5, MPI_INT, 2, 4, MPI_COMM_WORLD, &req3);
        MPI_Irecv(recv2, 5, MPI_INT, 2, 5, MPI_COMM_WORLD, &req4);
        
        // Signal that receives are posted
        int ready = 1;
        MPI_Send(&ready, 1, MPI_INT, 2, 13, MPI_COMM_WORLD);
        
        // Wait for receives to complete
        MPI_Wait(&req3, MPI_STATUS_IGNORE);
        MPI_Wait(&req4, MPI_STATUS_IGNORE);
        
        printf("Process 0: Received from immediate sends\n");
        printf("Process 0: First data: %d, %d, %d\n", recv1[0], recv1[1], recv1[2]);
        printf("Process 0: Second data: %d, %d, %d\n", recv2[0], recv2[1], recv2[2]);
        
        // Send back using immediate buffered send
        int bufsize = MPI_BSEND_OVERHEAD + 5 * sizeof(int);
        char* buffer = (char*)malloc(bufsize);
        MPI_Buffer_attach(buffer, bufsize);
        
        MPI_Ibsend(send_data, 5, MPI_INT, 2, 6, MPI_COMM_WORLD, &req3);
        MPI_Wait(&req3, MPI_STATUS_IGNORE);
        
        MPI_Buffer_detach(&buffer, &bufsize);
        free(buffer);
        
        printf("Process 0: Immediate buffered send completed\n");
    }
    
    // Final barrier
    MPI_Barrier(MPI_COMM_WORLD);
    
    if (rank == 0) {
        printf("\nAll communications completed successfully!\n");
    }
    
    MPI_Finalize();
    return 0;
}