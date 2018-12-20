/*
 *  Last name: Angulo
 *  First name: Valerie
 *  Net ID: N14591814 
 *  if you use cuda2: nvcc -o heatdist -arch=sm_52 heatdist.cu
 *  if you use cuda5: nvcc -o heatdist -arch=sm_35 heatdist.cu
*/

#include <cuda.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//define CUDA error checking
#define CUDA_ERROR_CHECK
#define CudaSafeCall( err ) __cudaSafeCall( err, __FILE__, __LINE__ )
#define CudaCheckError()    __cudaCheckError( __FILE__, __LINE__ )

//for cudamalloc
inline void __cudaSafeCall( cudaError err, const char *file, const int line )
{
#ifdef CUDA_ERROR_CHECK
    if (cudaSuccess != err) {
        fprintf(stderr, "cudaSafeCall() failed at %s:%i : %s\n", file, line, cudaGetErrorString(err)); 
        exit(1);
    }
#endif
    return;
}

//for kernel
inline void __cudaCheckError( const char *file, const int line )
{
#ifdef CUDA_ERROR_CHECK
    cudaError err = cudaGetLastError();
    if (cudaSuccess != err) { 
        fprintf(stderr, "cudaCheckError() failed at %s:%i : %s\n", file, line, cudaGetErrorString(err));
        exit(1);
    }
    // More careful checking. However, this will affect performance.
    //err = cudaDeviceSynchronize();
    //if(cudaSuccess != err) {
    //    fprintf(stderr, "cudaCheckError() with sync failed at %s:%i : %s\n", file, line, cudaGetErrorString(err));
    //    exit(1);
    //}
#endif
    return;
}

__global__ void FindPrimes (int* d_numbers, int N) {
	int tx = blockIdx.x * blockDim.x + threadIdx.x;	

	tx = min(tx, ((N+3)/2)); //decreases branch divergence: instead of if (tx <= ((N+1)/2)), exclusive N+3/2
	for (int i = tx+1; i < N-1; i++) { //check threads in positions tx+1->N-2
		if (d_numbers[tx] != 1) {
			if (d_numbers[i] % d_numbers[tx] == 0) {
				d_numbers[i] = 1;
			}
		}
	}
}

int main(int argc, char * argv[]) {

	if (argc != 2) {fprintf(stderr, "Missing file argument. Exiting\n"); exit(1);}
	int N = atoi(argv[argc-1]);
	int upper_bound = N-1;
	
	if (N <= 2 || N > 10000000) {fprintf(stderr, "N must be > 2 and <= 10,000,000\n"); exit(1);}
	
	int* number_arr = (int*)calloc(N-1, sizeof(int));
	if(!number_arr) {fprintf(stderr, " Cannot allocate the array\n"); exit(1);}

	//populate array with ints 2->N in array indices 0->N-2
	int index_int = 2;
	for (int i=0; i<upper_bound; i++) {
		number_arr[i] = index_int;
		index_int++; 
	}

/////////////////////////////////DEVICE CODE////////////////////////////
	//declare and allocate mem for device vars
	int* d_numbers;
	int size = (N-1) * sizeof(int);
	int blocks_per_grid = ((N-2)/1024) + 1;
	CudaSafeCall(cudaMalloc((void**)&d_numbers, size));	

	//transfer data to device
	cudaMemcpy(d_numbers, number_arr, size, cudaMemcpyHostToDevice); 

	//setup kernel config
	dim3 dimGrid(blocks_per_grid, 1, 1); 
	dim3 dimBlock(1024, 1, 1);

	//call cuda kernel
	FindPrimes<<<dimGrid, dimBlock>>>(d_numbers, N);  
	CudaCheckError();

	//transfer results from d_numbers
	cudaMemcpy(number_arr, d_numbers, size, cudaMemcpyDeviceToHost);
///////////////////////////////////////////////////////////////////////// 

	FILE* fp;
	char filename[15]; //create string for file name
	sprintf(filename, "%d.txt", N);
	fp = freopen(filename, "w", stdout);

	//print array of primes to file
	for (int i = 0; i < upper_bound; i++) {
		if (number_arr[i] != 1) {
			printf("%d ", number_arr[i]);
		}
	}

	fclose(fp);
	cudaFree(d_numbers);
	free(number_arr);
}

