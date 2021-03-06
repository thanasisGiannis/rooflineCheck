//=========================================================================

// SAMPLE SOURCE CODE - SUBJECT TO THE TERMS OF END-USER LICENSE AGREEMENT FOR
// Intel® Advisor 2017.

// /* Copyright (C) 2010-2017 Intel Corporation. All Rights Reserved.
 
 // The source code, information and material ("Material") 
 // contained herein is owned by Intel Corporation or its 
 // suppliers or licensors, and title to such Material remains 
 // with Intel Corporation or its suppliers or licensors.
 // The Material contains proprietary information of Intel or 
 // its suppliers and licensors. The Material is protected by 
 // worldwide copyright laws and treaty provisions.
 // No part of the Material may be used, copied, reproduced, 
 // modified, published, uploaded, posted, transmitted, distributed 
 // or disclosed in any way without Intel's prior express written 
 // permission. No license under any patent, copyright or other
 // intellectual property rights in the Material is granted to or 
 // conferred upon you, either expressly, by implication, inducement, 
 // estoppel or otherwise. Any license under such intellectual 
 // property rights must be express and approved by Intel in writing.
 // Third Party trademarks are the property of their respective owners.
 // Unless otherwise agreed by Intel in writing, you may not remove 
 // or alter this notice or any other notice embedded in Materials 
 // by Intel or Intel's suppliers or licensors in any way.
 
// ========================================================================

//  Simple minded matrix multiply
#include <iostream>
#include <sys/time.h>
#include <stdlib.h>
#include <omp.h>
#include <mkl_cblas.h>
using namespace std;

//routine to initialize an array with data
void init_array(double row, double col, double off, int arrSize, double **array)
{
  int i,j;

  for (i=0; i<arrSize; i++) {
    for (j=0; j<arrSize; j++) {
      array[i][j] = row*i+col*j+off;
    }
  }
}


// routine to print out contents of small arrays
void print_array(char * name, int arrSize, double **array)
{
  int i,j;
	
  cout << endl << name << endl;
  for (i=0;i<arrSize;i++){
    for (j=0;j<arrSize;j++) {
      cout << "\t" << array[i][j];
    }
    cout << endl;
  }
}



// matrix multiply routine
void multiply_d1(int arrSize, double **aMatrix, double **bMatrix, double **product)
{
  for(int i=0;i<arrSize;i++) {
    for(int j=0;j<arrSize;j++) {
      double sum = 0;
      for(int k=0;k<arrSize;k++) {
        sum += aMatrix[i][k] * bMatrix[k][j];
      }
      product[i][j] = sum;
    }
  }
}


void multiply_d2(int arrSize, double **aMatrix, double **bMatrix, double **product)
{

	double sum=0;

	for(int i=0; i < arrSize; i++){
		for(int j=0 ; j< arrSize; j++) product[i][j]=0;
		
		for(int k=0; k < arrSize; k++){
				double alpha = aMatrix[i][k];
				for(int j=0; j < arrSize; j++){
	 				product[i][j] += alpha*bMatrix[k][j];
				}
			}
	}


}

void multiply_d3(int arrSize, double **aMatrix, double **bMatrix, double **product)
{
	/* blocking */
	int blockI = 50;
	int blockJ = 50;
	int blockK = 50;
	double *miniB = (double *)malloc(blockK*blockJ*sizeof(double));
	double *miniA = (double *)malloc(blockI*blockK*sizeof(double));
	double *miniproduct = (double *)malloc(blockI*blockJ*sizeof(double));

	double dot=0.0;
	double aa=0.0;
	for (int i=0; i<arrSize; i+=blockI)
		for(int j=0; j<arrSize; j+=blockJ){

			for(int ii=0;ii<blockI;ii++)
				for(int jj=0;jj<blockJ;jj++)
					miniproduct[ii*blockJ+jj] = 0.0;

			for(int k=0; k<arrSize; k+=blockK){

				/* pre-initializing block B */
				for(int kk=0;kk<blockK;kk++)
					for(int jj=0;jj<blockJ;jj++)
						miniB[kk*blockJ+jj]=bMatrix[k+kk][j+jj];
					
				/* pre-initializing block A */
				for(int ii=0;ii<blockI;ii++)
					for(int kk=0;kk<blockK;kk++)
						miniA[ii*blockK+kk]=aMatrix[i+ii][k+kk];

					
				for(int ii=0;ii<blockI;ii++)
					for(int kk=0;kk<blockK;kk++){
						aa=miniA[ii*blockK+kk];
						for(int jj=0;jj<blockJ;jj++)
							miniproduct[ii*blockJ+jj]+=aa*miniB[kk*blockJ+jj];	
					}
			}


			for(int ii=0;ii<blockI;ii++)
				for(int jj=0;jj<blockJ;jj++)
					product[i+ii][j+jj] = miniproduct[ii*blockJ+jj];
		}

	free(miniA);
	free(miniB);
	free(miniproduct);
}


void multiply_d4(int arrSize, double **aMatrix, double **bMatrix, double **product)
{
	/* blocking */
	/*
	int blockI = 200;
	int blockJ = 200;
	int blockK = 100;
*/
	int blockI = 100;
	int blockJ = 200;
	int blockK = 100;

	#pragma omp parallel
	{
	double *miniB = (double *)malloc(blockK*blockJ*sizeof(double));
	double *miniA = (double *)malloc(blockI*blockK*sizeof(double));
	double *miniproduct = (double *)malloc(blockI*blockJ*sizeof(double));
	double dot=0.0;
	double aa=0.0;


	#pragma omp for collapse(2)
	for (int i=0; i<arrSize; i+=blockI)
		for(int j=0; j<arrSize; j+=blockJ){

			for(int ii=0;ii<blockI;ii++)
				for(int jj=0;jj<blockJ;jj++)
					miniproduct[ii*blockJ+jj] = 0.0;

			for(int k=0; k<arrSize; k+=blockK){

				/* pre-initializing block B */
				for(int kk=0;kk<blockK;kk++)
					for(int jj=0;jj<blockJ;jj++)
						miniB[kk*blockJ+jj]=bMatrix[k+kk][j+jj];
					
				/* pre-initializing block A */
				for(int ii=0;ii<blockI;ii++)
					for(int kk=0;kk<blockK;kk++)
						miniA[ii*blockK+kk]=aMatrix[i+ii][k+kk];

					
				for(int ii=0;ii<blockI;ii++)
					for(int kk=0;kk<blockK;kk++){
						aa=miniA[ii*blockK+kk];
						for(int jj=0;jj<blockJ;jj++)
							miniproduct[ii*blockJ+jj]+=aa*miniB[kk*blockJ+jj];	
					}
			}


			for(int ii=0;ii<blockI;ii++)
				for(int jj=0;jj<blockJ;jj++)
					product[i+ii][j+jj] = miniproduct[ii*blockJ+jj];
		}
		

	free(miniA);
	free(miniB);
	free(miniproduct);
	}

}

void multiply_d5(int arrSize, double **aMatrix, double **bMatrix, double **product,
 struct timeval *startTime,struct timeval  *endTime)
{
	double dot;
	double miniaa;
	/* blocking */
	int blockI = 200;
	int blockJ = 200;
	int blockK = 100;

	#pragma omp parallel
	{
	double *miniA = (double *)malloc(blockI*blockK*sizeof(double));
	double *miniB = (double *)malloc(blockK*blockJ*sizeof(double));
	double *miniproduct = (double *)malloc(blockI*blockJ*sizeof(double));
	double dot;
	int i,j;
	
//	gettimeofday(startTime, NULL);

	#pragma omp for collapse(2) 
	for (i=0; i<arrSize; i+=blockI)
		for(j=0; j<arrSize; j+=blockJ){

			for(int ii=0;ii<blockI;ii++)
				for(int jj=0;jj<blockJ;jj++)
					miniproduct[ii*blockJ+jj] = 0.0;

			for(int k=0; k<arrSize; k+=blockK){

				/* pre-initializing block B */
				for(int kk=0;kk<blockK;kk++)
					for(int jj=0;jj<blockJ;jj++)
						miniB[kk*blockJ+jj]=bMatrix[k+kk][j+jj];
					
				/* pre-initializing block A */
				for(int ii=0;ii<blockI;ii++)
					for(int kk=0;kk<blockK;kk++)
						miniA[ii*blockK+kk]=aMatrix[i+ii][k+kk];

				cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
								blockI, blockJ, blockK,
								1,
								miniA, blockK,
								miniB, blockJ,
								1,
								miniproduct, blockJ);	
			}

			for(int ii=0;ii<blockI;ii++)
				for(int jj=0;jj<blockJ;jj++)
					product[i+ii][j+jj] = miniproduct[ii*blockJ+jj];
		}
//	gettimeofday(endTime, NULL);
	
	free(miniA);
	free(miniB);
	free(miniproduct);

	}
	

}

void multiply_d6(int arrSize, double **aMatrix, double **bMatrix, double **product,
 struct timeval *startTime,struct timeval  *endTime)
{

	double *A = (double *)malloc(arrSize*arrSize*sizeof(double));
	double *B = (double *)malloc(arrSize*arrSize*sizeof(double));
	double *C = (double *)malloc(arrSize*arrSize*sizeof(double));

	for(int i=0;i<arrSize;i++)
		for(int j=0;j<arrSize;j++){
			A[i*arrSize+j] = aMatrix[i][j];
			B[i*arrSize+j] = bMatrix[i][j];
		}

	gettimeofday(startTime, NULL);

	cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
					arrSize, arrSize, arrSize,
					1,
					A, arrSize,
					B, arrSize,
					0,
					C, arrSize);	

	gettimeofday(endTime, NULL);

	for(int i=0;i<arrSize;i++)
		for(int j=0;j<arrSize;j++)
			product[i][j] = C[i*arrSize+j];


	free(A);
	free(B);
	free(C);

}




int main(int argc, char*argv[])
{
  int num=0;

  if(argc !=2) {
    cerr << "Usage: 1_mmult_serial[_debug] arraySize [default is 1024].\n";
    num = 1024;
  } else {
    num = atoi(argv[1]);
    if (num < 2) {
      cerr << "Array dimensions must be greater than 1; setting it to 2. \n" << endl;
      num = 2;
    }
    if (num > 9000) {
      cerr << "Array dimensions must not be greater than 9000; setting it to 9000. \n" << endl;
      num = 9000;
    }
  }

  double** aMatrix = new double*[num];
  double** bMatrix = new double*[num];
  double** product = new double*[num];

  for (int i=0; i<num; i++) {
    aMatrix[i] = new double[num];
    bMatrix[i] = new double[num];
    product[i] = new double[num];
  }

// initialize the arrays with different data
  init_array(3,-2,1,num,aMatrix);
  init_array(-2,1,3,num,bMatrix);

// start timing the matrix multiply code
  cout << "Size: " << num <<  " X "<< num << endl;

  struct timeval startTime, endTime;

#if 1
  gettimeofday(&startTime, NULL);

	multiply_d4(num, aMatrix, bMatrix,product);
	
// stop timing the matrix multiply code
  gettimeofday(&endTime, NULL);
#else


	multiply_d6(num, aMatrix, bMatrix, product,
			 &startTime, &endTime);
#endif







// print simple test case of data to be sure multiplication is correct
  if (num <= 6) {
    print_array((char*)("aMatrix"), num, aMatrix);
    print_array((char*)("bMatrix"), num, bMatrix);
    print_array((char*)("product"), num, product);
  }

   if (num <= 6) {
 	multiply_d1(num, aMatrix, bMatrix, product);
  	print_array((char*)("product"), num, product);
  }
// print elapsed time
  double useconds = (endTime.tv_sec*1000000 + endTime.tv_usec) - (startTime.tv_sec*1000000 + startTime.tv_usec);
  double seconds = useconds/1000000;
  cout << endl << "Calculations took " << seconds << " sec.\n";

// cleanup
  for (int i=0; i<num; i++) {
    delete [] aMatrix[i];
    delete [] bMatrix[i];
    delete [] product[i];
  }

  delete [] aMatrix;
  delete [] bMatrix;
  delete [] product;

  return 0;
}

