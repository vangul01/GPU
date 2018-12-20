/* 
To do: 
		make arrays that need to be dynamic
		make function that solves prediction Y with the B outputs
		ln[Y/(1−Y)]=a+bX 
		make another array for random values X
make arg for input random file?
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h> 
#include <iostream>
#include <cstdlib> //for random numbers
#include <ctime> //base rand on computer time for new rand each time
#include <math.h>
// #include<bits/stdc++.h> 

using namespace std;
#define index(i, j, colNum)  ((i)*(colNum)) + (j)

#include "0header.h"	
#include "0matrixCalculation.cpp"

int main(int argc, char *argv[]) {
	srand(time(NULL)); //use rand()%10 for random bt 0-9

	if (argc != 3) {
		cout << "Improper number of arguments. \nArg 1: Number of Rows\nArg 2: Number of Columns" << endl; 
		exit(1);
	}

	//error check this
	string rowInput = argv[(argc - 2)];
	string colInput = argv[(argc - 1)];
	int rowNum = stoi(rowInput);
	int colNum = stoi(colInput);

/*	if(rowInput.is_integer) {
	 	//((int)stoi(rowInput) && (int)stoi(colInput)) 
		//rowNum = stoi(rowInput);
		//colNum = stoi(colInput);
		cout << "Woo! Check passes :D" << endl;
	} else {
		cout << "Error: Make sure Row and Column arguments are integers." << endl;
	}
*/	
	cout <<"Rows: " << rowNum << " Columns: " << colNum << endl;
	
	//populate X with random values between 0-9 
	int* X = (int*)malloc(sizeof(int)*rowNum*colNum); 
	for(int i = 0; i < rowNum; i++) {
		for(int j = 0; j < colNum; j++) {
			X[index(i, j, colNum)] = rand()%10;	
		}
	}

	//populate Y with random 0 or 1 value
	float* Y = (float*)malloc(sizeof(float)*rowNum); 
	for(int i = 0; i < rowNum; i++) {
		Y[i] = rand()%2;
	}

	//transpose X on X' and create new matrix
	int* transposeX = (int*)malloc(sizeof(int)*rowNum*colNum); 
	for(int i = 0; i < rowNum; i++) {
		for(int j = 0; j < colNum; j++) {
			transposeX[index(j,i, rowNum)] = X[index(i,j, colNum)];	
		}
	}
	cout << "\n";

	N = colNum;
	int tranXmulxMat[colNum*colNum];
	matMultiplication(transposeX, colNum, rowNum, X, rowNum, colNum, tranXmulxMat);
	cout<<"X\'-------->"<<endl;
	matrixPrint(transposeX, colNum, rowNum);
	cout<<"X ----------->"<<endl;
	matrixPrint(X, rowNum, colNum);
	cout<<"X\'X----------->"<<endl;
	display(tranXmulxMat, colNum, colNum);

	/////////////////////////////////////////////////////////////////////
	float inv[N*N]; // To store inverse of A[][] 

	cout<<"The inverse is --------->"<<endl;
	if (inverse(tranXmulxMat, inv)) 
		display(inv, colNum, colNum); 

	//cal (X'X)^-1*X'
	//------------
	float XtFloat[colNum*rowNum];
	matIntToFloat(transposeX, colNum, rowNum, XtFloat, colNum, rowNum);

	float resultM[colNum*rowNum];
	matMultiplFloat(inv, colNum, colNum, XtFloat, colNum, rowNum, resultM);

	cout<<"(X'X)^-1*X' ------------->"<<endl;
	display(resultM, colNum, rowNum);
	
	//cal (X'X)^-1*X'Y
	float finalResult[colNum];
	matMultiplFloat(resultM, colNum, rowNum, Y, rowNum, 1, finalResult);

	cout<<"final (X'X)^-1*X'Y ------------->"<<endl;
	display(finalResult, colNum, 1);

	// FIGURING OUT PREDICTION Y ////////////////////////////////////////////
	cout <<"\nThese are my B values: "; 
	//here make Y function...
	float* B = (float*)malloc(sizeof(float)*colNum);
	for(int i = 0; i < colNum; i++) {
		B[i] = finalResult[i];
		cout << B[i] << " ";
	}
	cout << endl;

	//create random X values to multiply with the B values to get Y values
	//Then do the weird formula on Y values
	cout <<"These are my random X values: "; 
	float* newX = (float*)malloc(sizeof(float)*colNum-1);
	for(int i = 1; i < colNum; i++) {
		newX[i] = rand()%10;
		cout << newX[i] << " ";
	}
	cout << endl;

	//now figure out Y by multiplying the Bs with the new random Xs
	//float* predictY = (float*)malloc(sizeof(float));
	float predictY = 0;
	float temp = 0;
	for (int i=1; i<colNum; i++) {
		temp += B[i] * newX[i];
	}
	//Y=b0+b1X1+b2X2+....bNXN
	predictY = B[0] + temp; 
	cout << "b0 + b1X1 + b2X2 + bNXN ----> Y = " << predictY << endl;
	//Now turn this Y into ln[Y/(1−Y)]=a+bX... ?
	predictY = predictY/(1-predictY); //will give a+bX+...
	cout << "Y/(1−Y) = " << predictY << endl;
	
	if (predictY >= 0) {
		predictY = log(predictY);
		cout << "ln[Y/(1−Y)] = " << predictY << endl;
	} else {
		cout << "Error: Y/(1−Y) cannot be negative, 0 or 1.\n";
	}

	return 0;
}
