/* matmult.c 
 *    Test program to do matrix multiplication on large arrays.
 *
 *    Intended to stress virtual memory system.
 *
 *    Ideally, we could read the matrices off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

#define Dim 	20	/* sum total of the arrays doesn't fit in 
			 * physical memory 
			 */

int A[Dim][128];
int B[Dim][128];
int C[Dim][128];

int
main()
{
    int i, j, k;

    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
	{
		for (j = 0; j < Dim; j++) 
		{
			A[j][i] = i;
			B[j][i] = j;
			C[j][i] = i+j;
		}
	}


    for (i = 0; i < Dim; i++)		/* then multiply them together */
	{
		for (j = 0; j < Dim; j++)
		{
			for (k = 0; k < Dim; k++)
			{
				C[i][j] += A[i][k] * B[k][j];
			}
		}
	}

    Exit(C[Dim-1][Dim-1]);		/* and then we're done */
}
