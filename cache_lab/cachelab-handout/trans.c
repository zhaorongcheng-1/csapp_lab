/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

// 61*67
void trans_9(int M, int N, int A[N][M], int B[M][N]);

// 32*32
void trans_3(int M, int N, int A[N][M], int B[M][N]);

// 64*64
void trans_7(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == 32)
        return trans_3(M, N, A, B);

    if (M == 64)
        return trans_7(M, N, A, B);

    if (M == 61)
        return trans_9(M, N, A, B);
    
   
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 







char trans_3desc[] = "Simple row-wise scan transpose 8*8 read all line, solution for 32*32 transpose";
void trans_3(int M, int N, int A[N][M], int B[M][N])
{
    int i;

    int step = 8;

    int r, l;

    int line0, line1, line2, line3, line4, line5, line6, line7;

    for (r = 0; r < N/step; r++)
    {
        for (l = 0; l < M/step; l++)
	{

	    for (i = 0; i < step; i++)
	    {

                // think:
		// after block divided, 
		// in cache, block A[i][i] will conflict with block A^T[i][i], 
		// for A[i][i] and A^T[i][i], there address gap is over 2^(5+5), causes map to cache conflict
		// so we read total block(32 bytes), to avoid this

                line0 = A[r*step + i][l*step + 0];
                line1 = A[r*step + i][l*step + 1];
                line2 = A[r*step + i][l*step + 2];
                line3 = A[r*step + i][l*step + 3];
                line4 = A[r*step + i][l*step + 4];
                line5 = A[r*step + i][l*step + 5];
                line6 = A[r*step + i][l*step + 6];
                line7 = A[r*step + i][l*step + 7];




		B[l*step + 0][r*step + i] = line0;
		B[l*step + 1][r*step + i] = line1;
		B[l*step + 2][r*step + i] = line2;
		B[l*step + 3][r*step + i] = line3;
		B[l*step + 4][r*step + i] = line4;
		B[l*step + 5][r*step + i] = line5;
		B[l*step + 6][r*step + i] = line6;
		B[l*step + 7][r*step + i] = line7;

		
	    
	    }

	}
    
    }
    
   
}










char trans_6desc[] = "Simple row-wise scan transpose 4*4 read all line, ";
void trans_6(int M, int N, int A[N][M], int B[M][N])
{
    int i;

    int step = 4;

    int r, l;

    int line0, line1, line2, line3;

    for (r = 0; r < N/step; r++)
    {
        for (l = 0; l < M/step; l++)
	{

	    for (i = 0; i < step; i++)
	    {

                // think:
		// after block divided, 
		// in cache, block A[i][i] will conflict with block A^T[i][i], 
		// for A[i][i] and A^T[i][i], there address gap is over 2^(5+5), causes map to cache conflict
		// so we read total block(32 bytes), to avoid this

                line0 = A[r*step + i][l*step + 0];
                line1 = A[r*step + i][l*step + 1];
                line2 = A[r*step + i][l*step + 2];
                line3 = A[r*step + i][l*step + 3];
                




		B[l*step + 0][r*step + i] = line0;
		B[l*step + 1][r*step + i] = line1;
		B[l*step + 2][r*step + i] = line2;
		B[l*step + 3][r*step + i] = line3;
		

		
	    
	    }

	}
    
    }
    
   
}


char trans_7desc[] = "Simple row-wise scan transpose 4*4 read all line, store A2 to B2, solution for 64*64 transpose";
void trans_7(int M, int N, int A[N][M], int B[M][N])
{
    int ar_step, ac_step;

    
    int r, c;

    int line0, line1, line2, line3;

    
    int tmp1, tmp2, tmp3, tmp4;

    ar_step = 8;
    ac_step = 8;

    int i;

    int step = 4;
    

    for (r = 0; r < N/ar_step; r++)
    {
        for (c = 0; c < M/ac_step; c++)
        {
	    // divid 8*8 block into 4 4*4 block to process;
	    // 8*8 => 4 4*4, call matrix A1, A2, A3, A4. is like this:
	    //            A1    A2
	    //            A3    A4
	    // after transpose to A^T, is like this:
	    //            B1    B2
	    //            B3    B4
	    // which Bi == Ai^T
	    {
                
		// transpose matrix A1, and store A2 to B2
	        for (i = 0; i < step; i++)
	        {
		    line0 = A[r*ar_step + i][c*ac_step + 0];
		    line1 = A[r*ar_step + i][c*ac_step + 1];
		    line2 = A[r*ar_step + i][c*ac_step + 2];
		    line3 = A[r*ar_step + i][c*ac_step + 3];

		    
		    tmp1 = A[r*ar_step + i][c*ac_step + 4];
		    tmp2 = A[r*ar_step + i][c*ac_step + 5];
		    tmp3 = A[r*ar_step + i][c*ac_step + 6];
		    tmp4 = A[r*ar_step + i][c*ac_step + 7];


		    B[c*ac_step + 0][r*ar_step + i] = line0;
		    B[c*ac_step + 1][r*ar_step + i] = line1;
		    B[c*ac_step + 2][r*ar_step + i] = line2;
		    B[c*ac_step + 3][r*ar_step + i] = line3;
		    
		    B[c*ac_step + 0][r*ar_step + i + step] = tmp1;
		    B[c*ac_step + 1][r*ar_step + i + step] = tmp2;
		    B[c*ac_step + 2][r*ar_step + i + step] = tmp3;
		    B[c*ac_step + 3][r*ar_step + i + step] = tmp4;
		    
		}


                // transpose matrix A3;
		// and move matrix B2 to B3;
		for (i = 0; i < step; i++)
		{
		    // read B2 by row
		    tmp1 = B[c*ac_step + i][r*ar_step + 0 + step];
		    tmp2 = B[c*ac_step + i][r*ar_step + 1 + step];
		    tmp3 = B[c*ac_step + i][r*ar_step + 2 + step];
		    tmp4 = B[c*ac_step + i][r*ar_step + 3 + step];
		    
		    // read A3 by column
		    line0 = A[r*ar_step + step + 0][c*ac_step + i];
		    line1 = A[r*ar_step + step + 1][c*ac_step + i];
		    line2 = A[r*ar_step + step + 2][c*ac_step + i];
		    line3 = A[r*ar_step + step + 3][c*ac_step + i];
		    

		    // store B2 by row
		    B[c*ac_step + i][r*ar_step + 0 + step] = line0;
		    B[c*ac_step + i][r*ar_step + 1 + step] = line1;
		    B[c*ac_step + i][r*ar_step + 2 + step] = line2;
		    B[c*ac_step + i][r*ar_step + 3 + step] = line3;
		    

		    // store B3 by row
		    B[c*ac_step + i + step][r*ar_step + 0] = tmp1;
		    B[c*ac_step + i + step][r*ar_step + 1] = tmp2;
		    B[c*ac_step + i + step][r*ar_step + 2] = tmp3;
		    B[c*ac_step + i + step][r*ar_step + 3] = tmp4;


		    
		}
#if 1
		// transpose A4;
		for (i = 0; i < step; i++)
		{
		    line0 = A[r*ar_step + step + i][c*ac_step + step + 0];
		    line1 = A[r*ar_step + step + i][c*ac_step + step + 1];
		    line2 = A[r*ar_step + step + i][c*ac_step + step + 2];
		    line3 = A[r*ar_step + step + i][c*ac_step + step + 3];

		    
		    
		    B[c*ac_step + step + 0][r*ar_step + step + i] = line0;
		    B[c*ac_step + step + 1][r*ar_step + step + i] = line1;
		    B[c*ac_step + step + 2][r*ar_step + step + i] = line2;
		    B[c*ac_step + step + 3][r*ar_step + step + i] = line3;
		    
		}
#endif
	    }
	
	}
    
    }
  
}





char trans_8desc[] = "Simple row-wise scan transpose 4*4 read all line, store A2 to B2, read B2 by column";
void trans_8(int M, int N, int A[N][M], int B[M][N])
{
    int ar_step, ac_step;

    
    int r, c;

    int line0, line1, line2, line3;

    
    int tmp1, tmp2, tmp3, tmp4;

    ar_step = 8;
    ac_step = 8;

    int i;

    int step = 4;
    

    for (r = 0; r < N/ar_step; r++)
    {
        for (c = 0; c < M/ac_step; c++)
        {
	    // divid 8*8 block into 4 4*4 block to process;
	    // 8*8 => 4 4*4, call matrix A1, A2, A3, A4. is like this:
	    //            A1    A2
	    //            A3    A4
	    // after transpose to A^T, is like this:
	    //            B1    B2
	    //            B3    B4
	    // which Bi == Ai^T
	    {
                
		// transpose matrix A1, and store A2 to B2
	        for (i = 0; i < step; i++)
	        {
		    line0 = A[r*ar_step + i][c*ac_step + 0];
		    line1 = A[r*ar_step + i][c*ac_step + 1];
		    line2 = A[r*ar_step + i][c*ac_step + 2];
		    line3 = A[r*ar_step + i][c*ac_step + 3];

		    
		    tmp1 = A[r*ar_step + i][c*ac_step + 4];
		    tmp2 = A[r*ar_step + i][c*ac_step + 5];
		    tmp3 = A[r*ar_step + i][c*ac_step + 6];
		    tmp4 = A[r*ar_step + i][c*ac_step + 7];


		    B[c*ac_step + 0][r*ar_step + i] = line0;
		    B[c*ac_step + 1][r*ar_step + i] = line1;
		    B[c*ac_step + 2][r*ar_step + i] = line2;
		    B[c*ac_step + 3][r*ar_step + i] = line3;
		    
		    B[c*ac_step + 0][r*ar_step + i + step] = tmp1;
		    B[c*ac_step + 1][r*ar_step + i + step] = tmp2;
		    B[c*ac_step + 2][r*ar_step + i + step] = tmp3;
		    B[c*ac_step + 3][r*ar_step + i + step] = tmp4;
		    
		}


                // transpose matrix A3;
		// and move matrix B2 to B3;
		for (i = 0; i < step; i++)
		{
		    // read A3 by row
		    line0 = A[r*ar_step + step + i][c*ac_step + 0];
		    line1 = A[r*ar_step + step + i][c*ac_step + 1];
		    line2 = A[r*ar_step + step + i][c*ac_step + 2];
		    line3 = A[r*ar_step + step + i][c*ac_step + 3];
		    
		    // read B2 by column
		    tmp1 = B[c*ac_step + 0][r*ar_step + i + step];
		    tmp2 = B[c*ac_step + 1][r*ar_step + i + step];
		    tmp3 = B[c*ac_step + 2][r*ar_step + i + step];
		    tmp4 = B[c*ac_step + 3][r*ar_step + i + step];
		    


		    // store B2 by row
		    B[c*ac_step + 0][r*ar_step + i + step] = line0;
		    B[c*ac_step + 1][r*ar_step + i + step] = line1;
		    B[c*ac_step + 2][r*ar_step + i + step] = line2;
		    B[c*ac_step + 3][r*ar_step + i + step] = line3;
		    

		    // store B3 by row
		    B[c*ac_step + 0 + step][r*ar_step + i] = tmp1;
		    B[c*ac_step + 1 + step][r*ar_step + i] = tmp2;
		    B[c*ac_step + 2 + step][r*ar_step + i] = tmp3;
		    B[c*ac_step + 3 + step][r*ar_step + i] = tmp4;


		    
		}
#if 1
		// transpose A4;
		for (i = 0; i < step; i++)
		{
		    line0 = A[r*ar_step + step + i][c*ac_step + step + 0];
		    line1 = A[r*ar_step + step + i][c*ac_step + step + 1];
		    line2 = A[r*ar_step + step + i][c*ac_step + step + 2];
		    line3 = A[r*ar_step + step + i][c*ac_step + step + 3];

		    
		    
		    B[c*ac_step + step + 0][r*ar_step + step + i] = line0;
		    B[c*ac_step + step + 1][r*ar_step + step + i] = line1;
		    B[c*ac_step + step + 2][r*ar_step + step + i] = line2;
		    B[c*ac_step + step + 3][r*ar_step + step + i] = line3;
		    
		}
#endif
	    }
	
	}
    
    }
  
}

char trans_9desc[] = "Simple row-wise scan transpose 17*17 read all line, solution for 61*67 transpose";
void trans_9(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    int step = 17;

    int r, l;

    int line0;

    for (r = 0; r < (N+step-1)/step; r++)
    {
        for (l = 0; l < (M+step-1)/step; l++)
	{

	    for (i = 0; i < step && (r*step + i) < N; i++)
	    {
	        for (j = 0; j < step && (l*step + j) < M; j++)
		{
		    line0 = A[r*step + i][l*step + j];
		    B[l*step + j][r*step + i] = line0;
		}
	    }

	}
    
    }
    
   
}



/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    // registerTransFunction(trans, trans_desc); 
    
    // registerTransFunction(trans_2, trans_2desc); 
    //registerTransFunction(trans_3, trans_3desc); 



    //registerTransFunction(trans_6, trans_6desc); 

    //registerTransFunction(trans_7, trans_7desc); 
    
    //registerTransFunction(trans_8, trans_8desc); 

    //registerTransFunction(trans_9, trans_9desc); 



    
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

