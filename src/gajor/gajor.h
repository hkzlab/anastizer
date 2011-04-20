#ifndef _GAJOR_HEADER_
#define _GAJOR_HEADER_

float **matrix(long nrl, long nrh, long ncl, long nch);
float **convert_matrix(float *a, long nrl, long nrh, long ncl, long nch);
void free_matrix(float **m, long nrl, long nrh, long ncl, long nch);
void gaussj(float **a, int n, float **b, int m);

#endif
