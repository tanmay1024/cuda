#include <stdio.h>

__global__ void whoami(void) {
    int block_id =
        blockIdx.x +
        blockIdx.y * gridDim.x +
        blockIdx.z * gridDim.x * gridDim.y;

    int block_offset = 
        block_id + 
        
}

int main(int argc, char **argv) {
    const int b_x = 2, b_y = 3, b_z = 4;
    const int t_x = 4, t_y = 4, t_z = 4;

    int blocks_per_grid = b_x * b_y * b_z;
    int threads_per_block = t_x * t_y * t_z;

    printf("%d blocks per grid, %d threads per block\n", blocks_per_grid, threads_per_block);

    dim3 blocksPerGrid(b_x, b_y, b_z);
    dim3 threadsPerBlock(t_x, t_y, t_z);

    whoami<<<blocksPerGrid, threadsPerBlock>>>();
    cudaDeviceSynchronize();
}   