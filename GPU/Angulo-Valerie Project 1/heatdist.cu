/*
 *  Please write your name and net ID below
 *  
 *  Last name: Angulo
 *  First name: Valerie
 *  Net ID: N14591814
 * 
 */


/* 
 * This file contains the code for doing the heat distribution problem. 
 * You do not need to modify anything except starting  gpu_heat_dist() at the bottom
 * of this file.
 * In gpu_heat_dist() you can organize your data structure and the call to your
 * kernel(s) that you need to write too. 
 * 
 * You compile with:
 * 		nvcc -o heatdist -arch=sm_60 heatdist.cu   
 */

#include <cuda.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h> 

/* To index element (i,j) of a 2D array stored as 1D */
#define index(i, j, N)  ((i)*(N)) + (j)

//define CUDA error checking
#define CUDA_ERROR_CHECK
#define CudaSafeCall( err ) __cudaSafeCall( err, __FILE__, __LINE__ )
#define CudaCheckError()    __cudaCheckError( __FILE__, __LINE__ )

/*****************************************************************/

// Function declarations: Feel free to add any functions you want.
void  seq_heat_dist(float *, unsigned int, unsigned int);
void  gpu_heat_dist(float *, unsigned int, unsigned int);

//for cudamalloc
inline void __cudaSafeCall( cudaError err, const char *file, const int line )
{
#ifdef CUDA_ERROR_CHECK
    if ( cudaSuccess != err )
    {
        fprintf( stderr, "cudaSafeCall() failed at %s:%i : %s\n",
                 file, line, cudaGetErrorString( err ) );
        exit( -1 );
    }
#endif
    return;
}

//for cuda kernel
inline void __cudaCheckError( const char *file, const int line )
{
#ifdef CUDA_ERROR_CHECK
    cudaError err = cudaGetLastError();
    if ( cudaSuccess != err )
    {
        fprintf( stderr, "cudaCheckError() failed at %s:%i : %s\n",
                 file, line, cudaGetErrorString( err ) );
        exit( -1 );
    }
#endif
    return;
}

__global__ void CalculateHeat (float* d_temp, float* d_result, unsigned int N)
{
  int tx = blockIdx.x * blockDim.x + threadIdx.x;

  //compute only non-edge cases and put into d_result 
  //thread is within bounds of playground dimensions 
  if (tx < N*N) 
  { 
    if (tx > 0 && tx < N-1) //threads that are not on the edges
    { 
        for (int i = 1; i < N-1; i++) //i's that are not on the edges
        { 
          d_result[index(tx, i, N)] = (d_temp[index(tx-1, i, N)] + d_temp[index(tx+1, i, N)] 
            + d_temp[index(tx, i-1, N)] + d_temp[index(tx, i+1, N)])/4.0;
        }
    }
  }
}

/*****************************************************************/
/**** Do NOT CHANGE ANYTHING in main() function ******/

int main(int argc, char * argv[])
{
  unsigned int N; /* Dimention of NxN matrix */
  int type_of_device = 0; // CPU or GPU
  int iterations = 0;
  int i;
  
  /* The 2D array of points will be treated as 1D array of NxN elements */
  float * playground; 
  
  // to measure time taken by a specific part of the code 
  double time_taken;
  clock_t start, end;
  
  if(argc != 4)
  {
    fprintf(stderr, "usage: heatdist num  iterations  who\n");
    fprintf(stderr, "num = dimension of the square matrix (50 and up)\n");
    fprintf(stderr, "iterations = number of iterations till stopping (1 and up)\n");
    fprintf(stderr, "who = 0: sequential code on CPU, 1: GPU execution\n");
    exit(1);
  }
  
  type_of_device = atoi(argv[3]);
  N = (unsigned int) atoi(argv[1]);
  iterations = (unsigned int) atoi(argv[2]);
 
  
  /* Dynamically allocate NxN array of floats */
  playground = (float *)calloc(N*N, sizeof(float));
  if( !playground )
  {
   fprintf(stderr, " Cannot allocate the %u x %u array\n", N, N);
   exit(1);
  }
  
  /* Initialize it: calloc already initalized everything to 0 */
  // Edge elements to 70F
  for(i = 0; i < N; i++)
    playground[index(0,i,N)] = 70;
    
  for(i = 0; i < N; i++)
    playground[index(i,0,N)] = 70;
  
  for(i = 0; i < N; i++)
    playground[index(i,N-1, N)] = 70;
  
  for(i = 0; i < N; i++)
    playground[index(N-1,i,N)] = 70;
  
  // from (0,10) to (0,30) inclusive are 100F
  for(i = 10; i <= 30; i++)
    playground[index(0,i,N)] = 100;
  
   // from (n-1,10) to (n-1,30) inclusive are 150F
  for(i = 10; i <= 30; i++)
    playground[index(N-1,i,N)] = 150;
  
  if( !type_of_device ) // The CPU sequential version
  {  
    start = clock();
    seq_heat_dist(playground, N, iterations);
    end = clock();
  }
  else  // The GPU version
  {
     start = clock();
     gpu_heat_dist(playground, N, iterations); 
     end = clock();    
  }
  
  
  time_taken = ((double)(end - start))/ CLOCKS_PER_SEC;
  
  printf("Time taken for %s is %lf\n", type_of_device == 0? "CPU" : "GPU", time_taken);
  
  free(playground);
  
  return 0;

}


/*****************  The CPU sequential version (DO NOT CHANGE THAT) **************/
void  seq_heat_dist(float * playground, unsigned int N, unsigned int iterations)
{
  // Loop indices
  int i, j, k;
  int upper = N-1;
  
  // number of bytes to be copied between array temp and array playground
  unsigned int num_bytes = 0;
  
  float * temp; 
  /* Dynamically allocate another array for temp values */
  /* Dynamically allocate NxN array of floats */
  temp = (float *)calloc(N*N, sizeof(float));
  if( !temp )
  {
   fprintf(stderr, " Cannot allocate temp %u x %u array\n", N, N);
   exit(1);
  }
  
  num_bytes = N*N*sizeof(float);
  
  /* Copy initial array in temp */
  memcpy((void *)temp, (void *) playground, num_bytes);
  
  for( k = 0; k < iterations; k++)
  {
    /* Calculate new values and store them in temp */
    for(i = 1; i < upper; i++)
      for(j = 1; j < upper; j++)
	temp[index(i,j,N)] = (playground[index(i-1,j,N)] + 
	                      playground[index(i+1,j,N)] + 
			      playground[index(i,j-1,N)] + 
			      playground[index(i,j+1,N)])/4.0;
  
			      
   			      
    /* Move new values into old values */ 
    memcpy((void *)playground, (void *) temp, num_bytes);
  }
  
}

/***************** The GPU version: Write your code here *********************/
/* This function can call one or more kernels if you want ********************/
void  gpu_heat_dist(float * playground, unsigned int N, unsigned int iterations)
{
  //declare and allocate mem for device vars
  float* d_temp;
  float* d_result;
  float* pointer;
  int size = N * N * sizeof(float);
  int blocks_per_grid = ((N*N)/512) + 1; //enough threads for each point in playground
  CudaSafeCall(cudaMalloc((void**)&d_temp, size));
  CudaSafeCall(cudaMalloc((void**)&d_result, size));

  //transfer data to device, both have playground now
  cudaMemcpy(d_temp, playground, size, cudaMemcpyHostToDevice);
  cudaMemcpy(d_result, playground, size, cudaMemcpyHostToDevice);

  //setup kernel config
  dim3 dimGrid(blocks_per_grid, 1, 1); //based on dimensions of playground
  dim3 dimBlock(512, 1, 1); //512 threads per block 

  //start of iteration loop and kernel execution
  for(int it = 0; it < iterations; ++it) {
    //call cuda kernel to update solution using old solution, put updated solution in d_result
    CalculateHeat<<<dimGrid, dimBlock>>>(d_temp, d_result, N);  

    //swap pointer to make temp get result, result becomes old 
    pointer = d_temp;
    d_temp = d_result;
    d_result = pointer;
  }

  //transfer results from d_temp that holds results to playground
  cudaMemcpy(playground, d_temp, size, cudaMemcpyDeviceToHost);

  //free mem
  cudaFree(d_temp);
  cudaFree(d_result);
}
