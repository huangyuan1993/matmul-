const char* dgemm_desc = "My awesome dgemm.";
#include <stdlib.h>
#ifndef BLOCK_SIZE
#define BLOCK_SIZE ((int) 80)
#endif
#define START_BLOCK_SIZE 1200
#define CONFLICT_SIZE 64

void basic_dgemm(const int lda, const int M, const int N, const int K,
                 const double* restrict A, const double* restrict B, double* restrict C)
{

	double* restrict smallA = (double*) malloc(M * K * sizeof(double));
	double* restrict smallB = (double*) malloc(N * K * sizeof(double));
	
    int i, j, k;
	for(k=0;k<K;++k){
		for(i=0;i<M;++i){
			smallA[k*M+i]=A[k*lda+i];
		}
	}
	for(j=0;j<N;++j){
		for(k=0;k<K;++k){
			smallB[j*K+k]=B[j*lda+k];
		}
	}
    for (j = 0; j < N; ++j) {
         for (i = 0; i < M; ++i){
            double cij = C[j*lda+i];
            for (k = 0; k < K; ++k) {
                cij += smallA[k*M+i] * smallB[j*K+k];
            }
            C[j*lda+i] = cij;
        }
    }
	free(smallA);
	free(smallB);
}

void do_block(const int lda,
              const double* restrict A, const double* restrict B, double* restrict C,
              const int i, const int j, const int k)
{
    const int M = (i+BLOCK_SIZE > lda? lda-i : BLOCK_SIZE);
    const int N = (j+BLOCK_SIZE > lda? lda-j : BLOCK_SIZE);
    const int K = (k+BLOCK_SIZE > lda? lda-k : BLOCK_SIZE);
    basic_dgemm(lda, M, N, K,
                A + i + k*lda, B + k + j*lda, C + i + j*lda);
}
void square_dgemm(const int M, const double* restrict A, const double* restrict B, double* restrict C)
{
	if((M<START_BLOCK_SIZE)){
		int i, j, k;
		for (i = 0; i < M; ++i) {
			for (j = 0; j < M; ++j) {
				double cij = C[j*M+i];
				for (k = 0; k < M; ++k)
					cij += A[k*M+i] * B[j*M+k];
				C[j*M+i] = cij;
			}
		}
		return;
	}
    const int n_blocks = M / BLOCK_SIZE + (M%BLOCK_SIZE? 1 : 0);
    int bi, bj, bk;
    for (bi = 0; bi < n_blocks; ++bi) {
        const int i = bi * BLOCK_SIZE;
        for (bj = 0; bj < n_blocks; ++bj) {
            const int j = bj * BLOCK_SIZE;
            for (bk = 0; bk < n_blocks; ++bk) {
                const int k = bk * BLOCK_SIZE;
                do_block(M, A, B, C, i, j, k);
            }
        }
    }
}

