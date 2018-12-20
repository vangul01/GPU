#include <stdlib.h>
#include <stdio.h>
#include <time.h> 
#include <iostream>
#include <cstdlib>
#include <math.h>
#include <fstream>
#include<sstream>
#include<string.h>
#include <numeric>
#include <cuda_runtime.h>
#include <ctime>

using namespace std;
#define index(i, j, colNum)  ((i)*(colNum)) + (j)

#include "0.header.h"
#include "0.matrixCalculation.cpp"
#include "0.CUDAmatCalculation.cpp"

int main(int argc, char *argv[]) {
	// double time_taken;
 //   clock_t start, end;


	int rowNum = atoi(argv[1]);
	int colNum = atoi(argv[2]);

	float * X = (float *) calloc(rowNum*colNum, sizeof(float));
	float * transposeX = (float *) calloc(colNum*rowNum, sizeof(float));

	float * Y = (float *) calloc(rowNum, sizeof(float));
	readFile(X, Y, rowNum, colNum);

	// display(X, rowNum, colNum);
	// display(Y, rowNum, 1);

	for(int i = 0; i < rowNum; i++) {
		for(int j = 0; j < colNum; j++) {
			transposeX[index(j,i, rowNum)] = X[index(i,j, colNum)];	
		}
	}

	
	N = colNum;
	float * tranXmulxMat = (float *) calloc(colNum*colNum, sizeof(float));
	// matMultiplFloat(transposeX, colNum, rowNum, X, rowNum, colNum, tranXmulxMat);
	cudaMatMultiCall(transposeX, colNum, rowNum, X, rowNum, colNum, tranXmulxMat);

	// cout<<"X\'-------->"<<endl;
	// display(transposeX, colNum, rowNum);

	// cout<<"X ----------->"<<endl;
	// display(X, rowNum, colNum);
	
	// cout<<"X\'X----------->"<<endl;
	// display(tranXmulxMat, colNum, colNum);

	// /////////////////////////////////////////////////////////////////////
	float inv[N*N]; // To store inverse of A[][] 

	// start = clock();

	// display(tranXmulxMat, colNum, colNum);

	mainInverseCuda(tranXmulxMat, colNum, inv);
	// display(inv, colNum, colNum);

	// cout<<"The inverse is --------->"<<endl;
	// if (inverse(tranXmulxMat, inv)) {
	// 	// display(inv, colNum, colNum); 
	// }

	// end = clock();
	//cal (X'X)^-1*X'
	// ------------

	float resultM[colNum*rowNum];
	// matMultiplFloat(inv, colNum, colNum, transposeX, colNum, rowNum, resultM);
	cudaMatMultiCall(inv, colNum, colNum, transposeX, colNum, rowNum, resultM);
	
	// cout<<"(X'X)^-1*X' ->"<<endl;
	// display(resultM, colNum, rowNum);
	
	//cal (X'X)^-1*X'Y
	// cout<<"(X'X)^-1*X'Y-->"<<endl;
	float finalResult[colNum];
	// matMultiplFloat(resultM, colNum, rowNum, Y, rowNum, 1, finalResult);
	cudaMatMultiCall(resultM, colNum, rowNum, Y, rowNum, 1, finalResult);

	cout<<"final (X'X)^-1*X'Y ->"<<endl;
	display(finalResult, colNum, 1);

	// time_taken = ((double)(end - start))/ CLOCKS_PER_SEC;
	// cout<<" single Core Taken->"<<time_taken<<endl;
	
	return 0;
}
