#define CUDA_CHECK_RETURN(value) CheckCudaErrorAux(__FILE__,__LINE__, #value, value)
#define random(x) (rand()%x)


static void CheckCudaErrorAux (const char *file, unsigned line, const char *statement, cudaError_t err){
	if (err == cudaSuccess)
		return;
	std::cerr << statement<<" returned " << cudaGetErrorString(err) << "("<<err<< ") at "<<file<<":"<<line << std::endl;
	exit (1);
}


void printMatrix(float* inputMatrix, const int rows, const int cols){
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            std::cout << inputMatrix[i * cols + j] << "\t";
        }
        std::cout << std::endl;
    }
}


__global__ void harnessZeroKernel(float *d_augmentedMatrix, const int rowId1, const int rowId2, const int size) {
	__shared__ float blockR1[512];
	__shared__ float blockR2[512];
	const int tIdx = threadIdx.x;
	const int bIdx = blockIdx.x;
	const int colI = blockIdx.x * blockDim.x + threadIdx.x;
	if (colI < size * 2) {
		blockR1[tIdx] = d_augmentedMatrix[rowId1 * 2 * size + blockDim.x * bIdx + tIdx];
		blockR2[tIdx] = d_augmentedMatrix[rowId2 * 2 * size + blockDim.x * bIdx + tIdx];
		__syncthreads();
		d_augmentedMatrix[rowId1 * 2 * size + blockDim.x * bIdx + tIdx] = blockR1[tIdx] + blockR2[tIdx];
	}
}

__global__ void computeRowsKernel(float *d_augmentedMatrix, const int rowId, const int size) {
	__shared__ float blockR[512];
	__shared__ float Aii;
	const int tIdx = threadIdx.x;
	const int bIdx = blockIdx.x;
	const int colI = blockIdx.x * blockDim.x + threadIdx.x;
	if (colI < size * 2) {
		blockR[tIdx] = d_augmentedMatrix[rowId * 2 * size + blockDim.x * bIdx + tIdx];
		Aii = d_augmentedMatrix[rowId * 2 * size + rowId];
		__syncthreads();
		blockR[tIdx] = blockR[tIdx] / Aii;
		d_augmentedMatrix[rowId * 2 * size + blockDim.x * bIdx + tIdx] = blockR[tIdx];
	}
}

__global__ void computeColsKernel(float *d_augmentedMatrix, const int colId, const int size) {
	__shared__ float blockC[16][16];    // which col need to be zero
	__shared__ float blockCCurent[16][16];   // which col is the current col
	__shared__ float ARow[16];        // the pivot row
	const int tIdx = threadIdx.x;
	const int tIdy = threadIdx.y;
	const int rowI = blockIdx.y * blockDim.y + threadIdx.y;
	const int colI = blockIdx.x * blockDim.x + threadIdx.x;
	if (colI < size * 2 && rowI < size) {
		blockC[tIdy][tIdx] = d_augmentedMatrix[rowI * size * 2 + colId];
		if (blockC[tIdy][tIdx] != 0) {
			blockCCurent[tIdy][tIdx] = d_augmentedMatrix[rowI * size * 2 + colI];
			ARow[tIdx] = d_augmentedMatrix[colId * size * 2 + colI];
			__syncthreads();
			if (rowI != colId) {   // current row can't sub by current row
				blockCCurent[tIdy][tIdx] = blockCCurent[tIdy][tIdx] - blockC[tIdy][tIdx] * ARow[tIdx];
			}
			d_augmentedMatrix[rowI * size * 2 + colI] = blockCCurent[tIdy][tIdx];
			//d_augmentedMatrix[rowI * size * 2 + colI] = ARow[tIdx];
		}
	}
}

__global__ void augmentMatrixKernel(float *d_augmentedMatrix, float *d_inputMatrix, const int rows, const int cols) {
	const int rowI = blockIdx.y * blockDim.y + threadIdx.y;
	const int colI = blockIdx.x * blockDim.x + threadIdx.x;

	if (colI < cols && rowI < rows) {
			// initialize augmentedMatrix
			if (colI < cols / 2) {
				d_augmentedMatrix[rowI * cols + colI] = d_inputMatrix[rowI * cols / 2 + colI];
			}
			else if (colI - cols / 2 == rowI) {
				d_augmentedMatrix[rowI * cols + colI] = 1;
			} else {
				d_augmentedMatrix[rowI * cols + colI] = 0;
			}

	}
}

__global__ void getInverseMatrixKernel(float *d_augmentedMatrix, float *d_inverseMatrix, const int rows, const int cols) {
	const int rowI = blockIdx.y * blockDim.y + threadIdx.y;
	const int colI = blockIdx.x * blockDim.x + threadIdx.x;

	if (colI < cols / 2 && rowI < rows) {
			// initialize augmentedMatrix
			d_inverseMatrix[rowI * cols / 2 + colI] = d_augmentedMatrix[rowI * cols + colI + cols / 2];
	}
}


float *gpuMatrixInverse(float *inputMatrix, const int rows, const int cols){
	float *h_inverseMatrix;
	//float *h_augmentedMatrix;
	float *d_inputMatrix;
	float *d_inverseMatrix;
	float *d_augmentedMatrix;
	const int length = rows * cols;
	const int size = rows;
	//printMatrix(inputMatrix, rows, cols);
	cout << endl;
	// initialization
	h_inverseMatrix = (float *)malloc(length * sizeof(float));
	//h_augmentedMatrix = (float *)malloc(length * 2 * sizeof(float));
	CUDA_CHECK_RETURN(cudaMalloc((void **)&d_augmentedMatrix, sizeof(float) * length * 2));
	CUDA_CHECK_RETURN(cudaMalloc((void **)&d_inputMatrix, sizeof(float) * length));
	CUDA_CHECK_RETURN(cudaMalloc((void **)&d_inverseMatrix, sizeof(float) * length));
	CUDA_CHECK_RETURN(cudaMemcpy(d_inputMatrix, inputMatrix, sizeof(float) * length, cudaMemcpyHostToDevice));


	dim3 blockSize1(16, 16);
	dim3 gridSize1(cols * 2.0 / blockSize1.x + 1, rows * 1.0 / blockSize1.y + 1);
	augmentMatrixKernel<<<gridSize1, blockSize1>>>(d_augmentedMatrix, d_inputMatrix, rows, cols * 2);
	cudaDeviceSynchronize();

	int i = 0;
	while (i < size) {
		if (inputMatrix[i * size + i] != 0) {
			dim3 blockSize2(256);
			dim3 gridSize2(cols * 2.0 / blockSize2.x + 1, 1);
			computeRowsKernel<<<gridSize2, blockSize2>>>(d_augmentedMatrix, i, size);
			cudaDeviceSynchronize();
		} else {
			int nonZeroRowIndex = 0;
			for (int j = 0; j < size; j++) {
				if (inputMatrix[j * size + i] != 0) {
					nonZeroRowIndex = j;
					break;
				}
			}
			dim3 blockSize3(256);
			dim3 gridSize3(cols * 2.0 / blockSize3.x + 1, 1);
			harnessZeroKernel<<<gridSize3, blockSize3>>>(d_augmentedMatrix, i, nonZeroRowIndex, size);
			cudaDeviceSynchronize();
			dim3 blockSize4(256);
			dim3 gridSize4(cols * 2.0 / blockSize4.x + 1, 1);
			computeRowsKernel<<<gridSize4, blockSize4>>>(d_augmentedMatrix, i, size);
			cudaDeviceSynchronize();
		}

		dim3 blockSize5(16, 16);
		dim3 gridSize5(cols * 2.0 / blockSize5.x + 1, rows * 1.0 / blockSize5.y + 1);
		computeColsKernel<<<gridSize5, blockSize5>>>(d_augmentedMatrix, i, size);
		cudaDeviceSynchronize();
		i++;
	}

	dim3 blockSize6(16, 16);
	dim3 gridSize6(cols * 2.0 / blockSize6.x + 1, rows * 1.0 / blockSize6.y + 1);
	getInverseMatrixKernel<<<gridSize1, blockSize1>>>(d_augmentedMatrix, d_inverseMatrix, rows, cols * 2);

	CUDA_CHECK_RETURN(cudaMemcpy(h_inverseMatrix, d_inverseMatrix, sizeof(float) * length, cudaMemcpyDeviceToHost));
	//CUDA_CHECK_RETURN(cudaMemcpy(h_augmentedMatrix, d_augmentedMatrix, sizeof(float) * length * 2, cudaMemcpyDeviceToHost));
	CUDA_CHECK_RETURN(cudaFree(d_augmentedMatrix));
	CUDA_CHECK_RETURN(cudaFree(d_inverseMatrix));
	CUDA_CHECK_RETURN(cudaFree(d_inputMatrix));

	return h_inverseMatrix;
}

void mainInverseCuda(float mat[], int colNum, float resultMat[]){
	const int rows = colNum;
	const int cols = colNum;
	// float *testMatrix = createTest(rows, cols);
	float *testMatrix = mat;
	float *inverseMatrixGPU;

	// clock_t start1,end1;
	// start1 = clock();
	inverseMatrixGPU = gpuMatrixInverse(testMatrix, rows, cols);
	// end1 = clock();
	// float dur1 = (float)(end1 - start1);
	// cout << "\n running time on GPU is " << dur1 / CLOCKS_PER_SEC << " secs！\n" << endl;
	for(int i = 0; i < colNum*colNum; i++) {
		resultMat[i] = inverseMatrixGPU[i];
	}

	/* Free memory */
	delete[] inverseMatrixGPU;

}

__global__ void cudaMatMultiplication(float *matA, int rowA, int colA, float *matB , int rowB, int colB, float * resultMat) {
	// int i = threadIdx.x;
	// int j = threadIdx.y;
	int i = blockIdx.x*blockDim.x + threadIdx.x;
	int j = blockIdx.y*blockDim.y + threadIdx.y;

	if(i < rowA && j < colB ) {
		float p_val = 0.0;
		int k;
		for(k = 0; k < colA; k++) {
			float M_elm = matA[index(i, k, colA)];
			float N_elm = matB[index(k, j, colB)];
			p_val += M_elm*N_elm;
		// printf("%s%d %s%d %s%d %s%d \n", "i->",i, "j->",j, "rowA->",rowA, "colB->", colB );
		}
		
		resultMat[index(i, j, colB)] = p_val;
	} //end of if
	// __syncthreads();
}

void cudaMatMultiCall(float * matA, int rowA, int colA, float * matB , int rowB, int colB, float *resultMat) {
	size_t matAsize, matBsize, resultMatSize;
	matAsize = rowA*colA*sizeof(float); matBsize = rowB*colB*sizeof(float);resultMatSize = rowA*colB*sizeof(float);

	float * d_matA, * d_matB, * d_resultMat;
	cudaMalloc((void **)&d_matA, matAsize); cudaMalloc((void **)&d_matB, matBsize);
	cudaMalloc((void **)&d_resultMat, resultMatSize);

	cudaMemcpy(d_matA, matA, matAsize, cudaMemcpyHostToDevice);
	cudaMemcpy(d_matB, matB, matBsize, cudaMemcpyHostToDevice);
	
	dim3 dimBlock(32, 32);
	dim3 dimGrid(1024, 1024);
	// int aa =rowA/32; int bb = colB/32;
	// dim3 dimGrid(aa, bb);
	// dim3 dimGrid(rowA/32, colB/32);
	// int blockNum = (rowB + 1024 -1)/1024;
	// dim3 dimBlock(32, 32);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	cudaMatMultiplication<<<dimGrid, dimBlock>>>(d_matA, rowA, colA, d_matB, rowB, colB, d_resultMat);
	// cudaMatMultiplication<<<blockNum, dimBlock>>>(d_matA, rowA, colA, d_matB, rowB, colB, d_resultMat);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	float *h_resultMat = (float *)malloc(resultMatSize);
	cudaMemcpy(h_resultMat, d_resultMat, resultMatSize, cudaMemcpyDeviceToHost);

	// int i1;
	// for(i1 = 0; i1 < rowA*colB; i1 ++ ) {
	// 	cout<<"h_resultMat->"<<h_resultMat[i1]<<" il->"<<i1 <<endl;
	// }

	cudaFree(d_matA);
	cudaFree(d_matB);
	cudaFree(d_resultMat);

	unsigned int num_bytes = rowA* colB* sizeof(float);
	memcpy((void *) resultMat, (void *) h_resultMat, num_bytes);

	// cout<<" I'am in cudaMatMultiCall----------------"<<endl;
}
