/* 
http://www.appentra.com/parallel-matrix-matrix-multiplication 
https://computing.llnl.gov/tutorials/openMP/samples/C/omp_mm.c
*/

#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <math.h>

typedef double TYPE;

// Main functionalities
double sequentialMultiply(TYPE** matrixA, TYPE** matrixB, TYPE** result, int dimension);
double parallelMultiply(TYPE** matrixA, TYPE** matrixB, TYPE** result, int dimension);
double optimizedParallelMultiply(TYPE** matrixA, TYPE** matrixB, TYPE** result, int dimension);
TYPE** randomMatrix(int dimension);
TYPE** zeroMatrix(int dimension);
TYPE* zeroFlatMatrix(int dimension);
TYPE* rowMajor(TYPE** matrix, int dimension);
TYPE* columnMajor(TYPE** matrix, int dimension);
void displayMatrix(TYPE** matrix, int dimension);
void displayFlatMatrix(TYPE* matrix, int dimension);
TYPE* copyOf(TYPE* matrix, int dimension);

// Performance tests
void sequentialMultiplyTest(int dimension, int iterations);
void parallelMultiplyTest(int dimension, int iterations);
void optimizedParallelMultiplyTest(int dimension, int iterations);

int main(int argc, char* argv[]){
	int test = strtol(argv[1], NULL, 10);
	int dimension = strtol(argv[2], NULL, 10);
	int iterations = strtol(argv[3], NULL, 10);

	if(test==0){
		FILE* fp;
		fp = fopen("SequentialMultiplyTest.txt", "w+");
		sequentialMultiplyTest(dimension, iterations);
		fclose(fp);
	} else if(test==1){
		FILE* fp;
		fp = fopen("ParallelMultiplyTest.txt", "w+");
		parallelMultiplyTest(dimension, iterations);
		fclose(fp);
	} else if(test==2){
		FILE* fp;
		fp = fopen("OptimizedParallelMultiplyTest.txt", "w+");
		optimizedParallelMultiplyTest(dimension, iterations);
		fclose(fp);
	}

	return 0;
}

/* Main functionalities */

double sequentialMultiply(TYPE** matrixA, TYPE** matrixB, TYPE** result, int dimension){
	int i, j, k;

	struct timeval t0, t1;
	gettimeofday(&t0, 0);

	/* Begin process */

    for (i=0; i<dimension; i++)
    {
        for (j=0; j<dimension; j++)
        {
            for (k=0; k<dimension; k++){
                result[i][j] += matrixA[i][k] * matrixB[k][j] ;
            }
        }
    }
    /* End process */

    gettimeofday(&t1, 0);
	double elapsed = (t1.tv_sec-t0.tv_sec) * 1.0f + (t1.tv_usec - t0.tv_usec) / 1000000.0f;

	return elapsed;
}

double parallelMultiply(TYPE** matrixA, TYPE** matrixB, TYPE** result, int dimension){
	int i, j, k;

	struct timeval t0, t1;
	gettimeofday(&t0, 0);

	/* Begin process */

	#pragma omp parallel shared(matrixA, matrixB, result) private(i, j, k)
	{ 
		#pragma omp for schedule(static)
	    for (i=0; i<dimension; i++)
	    {
	        for (j=0; j<dimension; j++)
	        {	
	            for (k=0; k<dimension; k++){
	                result[i][j] += matrixA[i][k] * matrixB[k][j];
	            }
	        }
	    }
	}

    /* End process */

    gettimeofday(&t1, 0);
	double elapsed = (t1.tv_sec-t0.tv_sec) * 1.0f + (t1.tv_usec - t0.tv_usec) / 1000000.0f;

	return elapsed;
}

double optimizedParallelMultiply(TYPE** matrixA, TYPE** matrixB, TYPE** result, int dimension){
	int i, j, k, n_thread, chunk;

	struct timeval t0, t1;
	gettimeofday(&t0, 0);

	/* Begining of process */

	TYPE* matrixFlatA = rowMajor(matrixA, dimension);
	TYPE* matrixFlatB = columnMajor(matrixB, dimension);

	#pragma omp parallel shared(matrixFlatA, matrixFlatB, result) private(i, j, k)
	{
		
		int size = dimension * dimension;
		TYPE* tempA = copyOf(matrixFlatA, size);
		TYPE* tempB = copyOf(matrixFlatB, size);

		n_thread = omp_get_num_threads();
		chunk = dimension / (n_thread * 5);
		#pragma omp for schedule(dynamic, chunk)
		for(i=0; i<dimension; i++){
			for(j=0; j<dimension; j++){
				register double tot = 0;
				for(k=0; k<dimension; k++){
					tot += tempA[dimension * i + k] * tempB[dimension * j + k];
				}
				result[i][j] = tot;
			}
		}

		free(tempA);
		free(tempB);
	}

	free(matrixFlatA);
	free(matrixFlatB);

	/* End of process*/

	gettimeofday(&t1, 0);
	double elapsed = (t1.tv_sec-t0.tv_sec) * 1.0f + (t1.tv_usec - t0.tv_usec) / 1000000.0f;

	return elapsed;
}


TYPE** randomMatrix(int dimension){
	TYPE** matrix = malloc(dimension * sizeof(TYPE*));
	for(int i=0; i<dimension; i++){
		matrix[i] = malloc(dimension * sizeof(TYPE));
	}

	//Random seed
	srandom(time(0)+clock()+random());

	for(int i=0; i<dimension; i++){
		for(int j=0; j<dimension; j++){
			matrix[i][j] = rand() % 1000 + 1;
		}
	}
	return matrix;
}

TYPE** zeroMatrix(int dimension){
	TYPE** matrix = malloc(dimension * sizeof(TYPE*));
	for(int i=0; i<dimension; i++){
		matrix[i] = malloc(dimension * sizeof(TYPE));
	}

	srandom(time(0)+clock()+random());
	for(int i=0; i<dimension; i++){
		for(int j=0; j<dimension; j++){
			matrix[i][j] = 0;
		}
	}
	return matrix;
}

TYPE* zeroFlatMatrix(int dimension){
	TYPE* matrix = malloc(dimension * dimension * sizeof(TYPE));

	srandom(time(0)+clock()+random());
	for(int i=0; i<dimension*dimension; i++){
		matrix[i] = 0;
	}
	return matrix;
}


TYPE* rowMajor(TYPE** matrix, int dimension){
	TYPE* flatMatrix = malloc(dimension * dimension * sizeof(TYPE));
	for(int i=0; i<dimension; i++){
		for(int j=0; j<dimension; j++){
			flatMatrix[i * dimension + j] = matrix[i][j];
		}
	}
	return flatMatrix;
}

TYPE* columnMajor(TYPE** matrix, int dimension){
	TYPE* flatMatrix = malloc(dimension * dimension * sizeof(TYPE));
	for(int i=0; i<dimension; i++){
		for(int j=0; j<dimension; j++){
			flatMatrix[j * dimension + i] = matrix[i][j];
		}
	}
	return flatMatrix;
}

void displayMatrix(TYPE** matrix, int dimension){
	for(int i=0; i<dimension; i++){
		for(int j=0; j<dimension; j++){
			printf("%f\t", matrix[i][j]);
		}
		printf("\n");
	}
}

void displayFlatMatrix(TYPE* matrix, int dimension){
	for(int i=0; i<dimension; i++){
		for(int j=0; j<dimension; j++){
			printf("%f\t", matrix[dimension * i + j]);
		}
		printf("\n");
	}
}

TYPE* copyOf(TYPE* matrix, int dimension){
	TYPE* copy = malloc(dimension * sizeof(TYPE));
	for(int i=0; i<dimension; i++){
		copy[i] = matrix[i];
	}
	return copy;
}

/* Performance tests */

void sequentialMultiplyTest(int dimension, int iterations){
	FILE* fp;
	fp = fopen("SequentialMultiplyTest.txt", "a+");

	// Console write
	printf("----------------------------------\n");
	printf("Test : Sequential Multiply        \n");
	printf("----------------------------------\n");
	printf("Dimension : %d\n", dimension);
	printf("..................................\n");
	
	// File write
	fprintf(fp, "----------------------------------\n");
	fprintf(fp, "Test : Sequential Multiply        \n");
	fprintf(fp, "----------------------------------\n");
	fprintf(fp, "Dimension : %d\n", dimension);
	fprintf(fp, "..................................\n");

	double* opmLatency = malloc(iterations * sizeof(double));
	TYPE** matrixA = randomMatrix(dimension);
	TYPE** matrixB = randomMatrix(dimension);
	
	// Iterate and measure performance
	for(int i=0; i<iterations; i++){
		TYPE** matrixResult = zeroMatrix(dimension);
		opmLatency[i] = sequentialMultiply(matrixA, matrixB, matrixResult, dimension);
		free(matrixResult);

		// Console write
		printf("%d.\t%f\n", i+1, opmLatency[i]);

		// File write
		fprintf(fp, "%d.\t%f\n", i+1, opmLatency[i]);
	}

	// Console write
	printf("\n");
	printf("----------------------------------\n");
	printf("Analyze Measurements              \n");
	printf("----------------------------------\n");

	// File write
	fprintf(fp, "\n");
	fprintf(fp, "----------------------------------\n");
	fprintf(fp, "Analyze Measurements              \n");
	fprintf(fp, "----------------------------------\n");

	double sum = 0.0;
	double sumSquared = 0.0;

	// Statistical analyze
	for(int i=0; i<iterations; i++){
		sum += opmLatency[i];
		sumSquared += pow(opmLatency[i], 2.0);
	}

	double mean = sum / iterations;
	double squareMean = sumSquared / iterations;
	double standardDeviation = sqrt(squareMean - pow(mean, 2.0));

	// Console write
	printf("Mean               : %f\n", mean);
	printf("Standard Deviation : %f\n", standardDeviation);
	printf("----------------------------------\n");

	//File write
	fprintf(fp, "Mean               : %f\n", mean);
	fprintf(fp, "Standard Deviation : %f\n", standardDeviation);
	fprintf(fp, "----------------------------------\n");

	// Releasing memory
	fclose(fp);
	free(opmLatency);
	free(matrixA);
	free(matrixB);
}

void parallelMultiplyTest(int dimension, int iterations){
	FILE* fp;
	fp = fopen("ParallelMultiplyTest.txt", "a+");

	// Console write
	printf("----------------------------------\n");
	printf("Test : Parallel Multiply          \n");
	printf("----------------------------------\n");
	printf("Dimension : %d\n", dimension);
	printf("..................................\n");
	
	// File write
	fprintf(fp, "----------------------------------\n");
	fprintf(fp, "Test : Parallel Multiply          \n");
	fprintf(fp, "----------------------------------\n");
	fprintf(fp, "Dimension : %d\n", dimension);
	fprintf(fp, "..................................\n");

	double* opmLatency = malloc(iterations * sizeof(double));
	TYPE** matrixA = randomMatrix(dimension);
	TYPE** matrixB = randomMatrix(dimension);
	
	// Iterate and measure performance
	for(int i=0; i<iterations; i++){
		TYPE** matrixResult = zeroMatrix(dimension);
		opmLatency[i] = parallelMultiply(matrixA, matrixB, matrixResult, dimension);
		free(matrixResult);

		// Console write
		printf("%d.\t%f\n", i+1, opmLatency[i]);

		// File write
		fprintf(fp, "%d.\t%f\n", i+1, opmLatency[i]);
	}

	// Console write
	printf("\n");
	printf("----------------------------------\n");
	printf("Analyze Measurements              \n");
	printf("----------------------------------\n");

	// File write
	fprintf(fp, "\n");
	fprintf(fp, "----------------------------------\n");
	fprintf(fp, "Analyze Measurements              \n");
	fprintf(fp, "----------------------------------\n");

	double sum = 0.0;
	double sumSquared = 0.0;

	// Statistical analyze
	for(int i=0; i<iterations; i++){
		sum += opmLatency[i];
		sumSquared += pow(opmLatency[i], 2.0);
	}

	double mean = sum / iterations;
	double squareMean = sumSquared / iterations;
	double standardDeviation = sqrt(squareMean - pow(mean, 2.0));

	// Console write
	printf("Mean               : %f\n", mean);
	printf("Standard Deviation : %f\n", standardDeviation);
	printf("----------------------------------\n");

	//File write
	fprintf(fp, "Mean               : %f\n", mean);
	fprintf(fp, "Standard Deviation : %f\n", standardDeviation);
	fprintf(fp, "----------------------------------\n");

	// Releasing memory
	fclose(fp);
	free(opmLatency);
	free(matrixA);
	free(matrixB);
}

void optimizedParallelMultiplyTest(int dimension, int iterations){
	FILE* fp;
	fp = fopen("OptimizedParallelMultiplyTest.txt", "a+");

	// Console write
	printf("----------------------------------\n");
	printf("Test : Optimized Parallel Multiply\n");
	printf("----------------------------------\n");
	printf("Dimension : %d\n", dimension);
	printf("..................................\n");
	
	// File write
	fprintf(fp, "----------------------------------\n");
	fprintf(fp, "Test : Optimized Parallel Multiply\n");
	fprintf(fp, "----------------------------------\n");
	fprintf(fp, "Dimension : %d\n", dimension);
	fprintf(fp, "..................................\n");

	double* opmLatency = malloc(iterations * sizeof(double));
	TYPE** matrixA = randomMatrix(dimension);
	TYPE** matrixB = randomMatrix(dimension);
	
	// Iterate and measure performance
	for(int i=0; i<iterations; i++){
		TYPE** matrixResult = zeroMatrix(dimension);
		opmLatency[i] = optimizedParallelMultiply(matrixA, matrixB, matrixResult, dimension);
		free(matrixResult);

		// Console write
		printf("%d.\t%f\n", i+1, opmLatency[i]);

		// File write
		fprintf(fp, "%d.\t%f\n", i+1, opmLatency[i]);
	}

	// Console write
	printf("\n");
	printf("----------------------------------\n");
	printf("Analyze Measurements              \n");
	printf("----------------------------------\n");

	// File write
	fprintf(fp, "\n");
	fprintf(fp, "----------------------------------\n");
	fprintf(fp, "Analyze Measurements              \n");
	fprintf(fp, "----------------------------------\n");

	double sum = 0.0;
	double sumSquared = 0.0;

	// Statistical analyze
	for(int i=0; i<iterations; i++){
		sum += opmLatency[i];
		sumSquared += pow(opmLatency[i], 2.0);
	}

	double mean = sum / iterations;
	double squareMean = sumSquared / iterations;
	double standardDeviation = sqrt(squareMean - pow(mean, 2.0));

	// Console write
	printf("Mean               : %f\n", mean);
	printf("Standard Deviation : %f\n", standardDeviation);
	printf("----------------------------------\n");

	//File write
	fprintf(fp, "Mean               : %f\n", mean);
	fprintf(fp, "Standard Deviation : %f\n", standardDeviation);
	fprintf(fp, "----------------------------------\n");

	// Releasing memory
	fclose(fp);
	free(opmLatency);
	free(matrixA);
	free(matrixB);
}