bool inverse(int A[], float inverse[]);
float determinant(float A[], int n);
void getCofactor(float A[], float temp[], int p, int q, int n);
void adjoint(float A[], float adj[]);

template<class T> 
void display(T A[], int rowN, int colN) { 
	for (int i=0; i< rowN; i++) { 
		for (int j=0; j< colN; j++) 
			cout << A[index(i, j, colN)] << " "; 
		cout << endl; 
	} 
}

void matrixPrint(int mat[], int rowN, int colN) {
	for(int i = 0; i < rowN; i++) {
		for(int j = 0; j < colN; j++) {
			printf("%d ", mat[index(i, j, colN)] );
			if(j == colN -1) printf("\n");
		}
	}
}

////////////////////////////////////////////
/////////////////////////////////////////////
bool inverse(float A[], float inverse[]) { 
	float det = determinant(A, N); 

	if (det == 0.0) { 
		cout << "Singular matrix, can't find its inverse"; 
		return false; 
	} 

	float adj[N*N]; 

	adjoint(A, adj); 

	for (int i=0; i<N; i++) 
		for (int j=0; j<N; j++) 
			inverse[index(i,j,N)] = adj[index(i,j,N)]/float(det); 

	return true; 
}  
///
void adjoint(float A[], float adj[]) { 
	int sign = 1; 
	float temp[N*N]; 

	for (int i=0; i<N; i++) { 
		for (int j=0; j<N; j++) { 
			// Get cofactor of A[i][j] 
			getCofactor(A, temp, i, j, N); 

			sign = ((i+j)%2==0)? 1: -1; 

			adj[index(j,i,N)] = (sign)*(determinant(temp, N-1)); 
		} 
	} 
} 
////
float determinant(float A[], int n) {
	float D = 0.0; // Initialize result 

	// Base case : if matrix contains single element 
	if (n == 1) 
		return A[index(0,0,N)]; 

	float temp[N*N]; // To store cofactors 

	float sign = 1.0; // To store sign multiplier 

	// Iterate for each element of first row 
	for (int f = 0; f < n; f++) { 
		getCofactor(A, temp, 0, f, n); 
		D += sign * A[index(0,f,N)] * determinant(temp, n - 1); 

		// terms are to be added with alternate sign 
		sign = -sign; 
	} 

	return D; 
} 
//////
void getCofactor(float A[], float temp[], int p, int q, int n) { 
	int i = 0, j = 0; 

	for (int row = 0; row < n; row++) { 
		for (int col = 0; col < n; col++) { 

			if (row != p && col != q) { 
				
				temp[index(i,j,N)] = A[index(row,col,N)]; 
				j++;

				if (j == n - 1) { 
					j = 0; 
					i++; 
				} 
			} 
		} 
	} 
}
/////////////////////////////////////////////////////////////
void matMultiplFloat(float matA[], int rowA, int colA, float matB[], int rowB, int colB, float resultMat[]) {
	float tempSum;

	for(int i = 0; i < rowA; i++) {
		for(int j = 0; j < colB; j++) {
			tempSum = 0;
			for(int k = 0; k < colA; k ++ ) {
				tempSum += matA[index(i, k, colA)]*matB[index(k, j, colB)] ;
				// cout<<"A*B ->"<<matA[index(i, k, colA)]<<"*"<<matB[index(k, j, colB)]<<endl;;
			}
			// tempMat[index(i, j, colB)] = tempSum;
			resultMat[index(i,j, colB)] = tempSum;
		}
	}
}

///////////////////////////////////////////////
void readFile(float X [], float Y[], int rowNum, int colNum) {
	char fileName[1000];
	sprintf(fileName, "%dby%dMat.txt", rowNum, colNum); 

	ifstream file(fileName);
	string line;
	int ii = 0;
	int rowI, colJ;
	float val;
	// float dataFloatArr[rowNum*colNum];
	while (std::getline(file, line )) {
		// cout<<"ii->"<<ii<<" ->"<<  line<<endl;
		istringstream ss(line);
		for(int i = 0; i < colNum; i++) {
			rowI = ii /colNum;
			colJ = ii % colNum;
			
			ss >>val;
			// dataFloatArr[ii] = val;

			if(colJ == 0) {
				Y[index(rowI, 0, 1)] = val;
				X[index(rowI, 0, colNum)] = 1.0;
			} else {
				X[index(rowI, colJ, colNum)] = val;
			}
			
			ii++;
		}
	} 	

	// display(Y, rowNum, 1);
	// cout<<"================="<<endl;
	// display(X, rowNum, colNum);

}




































