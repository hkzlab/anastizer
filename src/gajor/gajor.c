#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "gajor.h"

#define NR_END 1
#define FREE_ARG char*
#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}

void nrerror(char error_text[])
/* Numerical Recipes standard error handler */
{
	fprintf(stderr, "%s\n", error_text);
	exit(1);
}

int *ivector(long nl, long nh)
/* allocate an int vector with subscript range v[nl..nh] */
{
	int *v;

	v = (int *)malloc((size_t)((nh - nl + 1 + NR_END) * sizeof(int)));
	if (!v) nrerror("allocation failure in ivector()");
	return v - nl + NR_END;
}

void free_ivector(int *v, long nl, long nh)
/* free an int vector allocated with ivector() */
{
	free((FREE_ARG)(v + nl - NR_END));
}

float **create_nr_matrix(long nrl, long nrh, long ncl, long nch)
/* allocate a float matrix with subscript range m[nrl..nrh][ncl..nch] */
{
	long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
	float **m;

	/* allocate pointers to rows */
	m=(float **) malloc((size_t)((nrow+NR_END)*sizeof(float*)));
	if (!m) nrerror("allocation failure 1 in matrix()");
	m += NR_END;
	m -= nrl;

	/* allocate rows and set pointers to them */
	m[nrl]=(float *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(float)));
	if (!m[nrl]) nrerror("allocation failure 2 in matrix()");
	m[nrl] += NR_END;
	m[nrl] -= ncl;

	for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

	/* return pointer to array of pointers to rows */
	return m;
}

float **matrix_nr_product(float **m1, long nrl1, long nrh1, long ncl1, long nch1, float **m2, long nrl2, long nrh2, long ncl2, long nch2) {
	assert(m1);
	assert(m2);
	assert(nch1 - ncl1 == nrh2 - nrl2);
	assert(nrl1 == 1 && ncl1 == 1 && nrl2 == 1 && ncl2 == 1);

	float **dm = create_nr_matrix(nrl1, nrh1, ncl2, nch2);

	unsigned int i, j, r;
	float sum;

	for (i = nrl1; i <= nrh1; i++ )
		for (j = ncl2; j <= nch2; j++) {
			sum = 0;
			for (r = ncl1; r <= nch1; r++) {
				sum += m1[i][r] * m2[r][j];
			}
			dm[i][j] = sum;
		}

	return dm;
}

float **convert_matrix_to_nr(float *a, long nrl, long nrh, long ncl, long nch)
/* allocate a float matrix m[nrl..nrh][ncl..nch] that points to the matrix
declared in the standard C manner as a[nrow][ncol], where nrow=nrh-nrl+1
and ncol=nch-ncl+1. The routine should be called with the address
&a[0][0] as the first argument. */
{
	long i,j,nrow=nrh-nrl+1,ncol=nch-ncl+1;
	float **m;

	/* allocate pointers to rows */
	m=(float **) malloc((size_t) ((nrow+NR_END)*sizeof(float*)));
	if (!m) nrerror("allocation failure in convert_matrix()");
	m += NR_END;
	m -= nrl;

	/* set pointers to rows */
	m[nrl]=a-ncl;
	for(i=1,j=nrl+1;i<nrow;i++,j++) m[j]=m[j-1]+ncol;
	/* return pointer to array of pointers to rows */
	return m;
}

float *convert_nr_to_matrix(float **mnr, long w, long h) {
	float *m = (float*)malloc(sizeof(float) * w * h);

	long i, j;

	for (i = 0; i < h; i++)
		for (j = 0; j < w; j++) {
			m[i * w + j] = mnr[i][j];
		}

	return m;
}

void free_nr_matrix(float **m, long nrl, long nrh, long ncl, long nch)
/* free a float matrix allocated by matrix() */
{
	free((FREE_ARG) (m[nrl]+ncl-NR_END));
	free((FREE_ARG) (m+nrl-NR_END));
}

void gaussj(float **a, int n, float **b, int m) {
	int *indxc, *indxr, *ipiv;
	int i, j, k, l, ll;
	int icol = 0;
	int irow = 0;
	float big, dum, pivinv, temp;
	indxc = ivector(1, n);
	indxr = ivector(1, n);
	ipiv = ivector(1, n);
	for (j = 1; j <= n; j++) ipiv[j] = 0;
	for (i = 1; i <= n; i++) {
		big = 0.0;
		for (j = 1; j <= n; j++)
			if (ipiv[j] != 1)
				for (k = 1; k <= n; k++) {
					if (ipiv[k] == 0) {
						if (fabs(a[j][k]) >= big) {
							big = fabs(a[j][k]);
							irow = j;
							icol = k;
						}
					}
				}
		++(ipiv[icol]);
		if (irow != icol) {
			for (l = 1; l <= n; l++) SWAP(a[irow][l], a[icol][l])
				for (l = 1; l <= m; l++) SWAP(b[irow][l], b[icol][l])
				}
		indxr[i] = irow;
		indxc[i] = icol;
		if (a[icol][icol] == 0.0) nrerror("gaussj: Singular Matrix");
		pivinv = 1.0 / a[icol][icol];
		a[icol][icol] = 1.0;
		for (l = 1; l <= n; l++) a[icol][l] *= pivinv;
		for (l = 1; l <= m; l++) b[icol][l] *= pivinv;
		for (ll = 1; ll <= n; ll++)
			if (ll != icol) {
				dum = a[ll][icol];
				a[ll][icol] = 0.0;
				for (l = 1; l <= n; l++) a[ll][l] -= a[icol][l] * dum;
				for (l = 1; l <= m; l++) b[ll][l] -= b[icol][l] * dum;
			}
	}
	for (l = n; l >= 1; l--) {
		if (indxr[l] != indxc[l])
			for (k = 1; k <= n; k++)
				SWAP(a[k][indxr[l]], a[k][indxc[l]]);
	}
	free_ivector(ipiv, 1, n);
	free_ivector(indxr, 1, n);
	free_ivector(indxc, 1, n);
}

float **create_ident_matrix(long nrl, long nrh, long ncl, long nch) {
	float **id = create_nr_matrix(nrl, nrh, ncl, nch);

	unsigned int mh, mw;

	for (mh = nrl; mh <= nrh; mh++)
		for (mw = ncl; mw <= nch; mw++) {
			if (mh != mw) 
				id[mh][mw] = 0;
			else
				id[mh][mw] = 1;
		}

	return id;
}

float **clone_nr_matrix(float **m, long nrl, long nrh, long ncl, long nch) {
	float **md = create_nr_matrix(nrl, nrh, ncl, nch);

	unsigned int mh, mw;

	for (mh = nrl; mh <= nrh; mh++)
		for (mw = ncl; mw <= nch; mw++) {
			md[mh][mw] = m[mh][mw];
		}

	return md;
}

float **get_transf_matrix(float **ma, float **mb, long n) {
	long nrh = n - 1;
	long nch = nrh;
	float **id = create_ident_matrix(1, nrh, 1, nch);
	float **mac = clone_nr_matrix(ma, 1, nrh, 1, nch);
	float **tm;

	gaussj(mac, n, id, n);

	tm = matrix_nr_product(mac, 1, nrh, 1, nch, mb, 1, nrh, 1, nch);
	
	free_nr_matrix(mac, 1, nrh, 1, nch);
	free_nr_matrix(id, 1, nrh, 1, nch);

	return tm;
}
