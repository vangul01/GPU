/*
 *  Last name: Angulo
 *  First name: Valerie
 *  Net ID: N14591814 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char * argv[]) {

	if (argc != 2) {fprintf(stderr, "Missing file argument. Exiting\n"); exit(1);}
	int N = atoi(argv[argc-1]);
	int floor = (N+1)/2;
	int upper_bound = N-1;
	
	if (N <= 2 || N > 10000000) {fprintf(stderr, "N must be > 2 and <= 10,000,000\n"); exit(1);}
	
	int* number_arr = (int*)calloc(N-1, sizeof(int));
	if(!number_arr) {fprintf(stderr, " Cannot allocate the array\n"); exit(1);}

	//populate array with ints 2->N in array indices 0->N-2
	int index_int = 2;
	for (int i = 0; i < upper_bound; i++) {	
		number_arr[i] = index_int;
		index_int++; 
	}

	//check for primes
	int ptr = 0;
	for (int i = ptr; i <= floor; i++) { //move pointer from 0->floor
		for (int j = ptr+1; j < upper_bound; j++) { //check numbers in array indices ptr+1->N-2
			if (number_arr[ptr] != 1) {
				if (number_arr[j] % number_arr[ptr] == 0) {
					number_arr[j] = 1;
				}
			}
		}
		ptr++;
	}

	FILE* fp;
	char filename[15]; //create string for file name
	sprintf(filename, "%d.txt", N);
	fp = freopen(filename, "w", stdout); //write to specified file

	//print array of primes to file
	for (int i = 0; i < upper_bound; i++) {
		if (number_arr[i] != 1) {
			printf("%d ", number_arr[i]);
		}
	}

	fclose(fp);
	free(number_arr);
}

/*
//check array is correct
//      for (int i=0; i<upper_bound; i++) {     //populate with numbers 2->N
//              printf("%d ",number_arr[i]);
//      }

int* prime_arr = (int*)calloc(arr_size - elem_to_remove, sizeof(int));
if(!prime_arr) {fprintf(stderr, " Cannot allocate the array\n"); exit(1);}
int prime_index = 0;
//populate an array of the primes
for (int i = 0; i < upper_bound; i++) {
	if (number_arr[i] != 1) {
		prime_arr[prime_index] = number_arr[i];
		prime_index++;
	}
}

//print array of primes to file
for (int i = 0; i < (arr_size - elem_to_remove); i++) {
	printf("%d ", prime_arr[i]);
}
*/
