#ifndef _GAJOR_HEADER_
#define _GAJOR_HEADER_

float **matrix_nr_product(float **m1, long nrl1, long nrh1, long ncl1, long nch1, float **m2, long nrl2, long nrh2, long ncl2, long nch2);
float **create_nr_matrix(long nrl, long nrh, long ncl, long nch);
float **convert_matrix_to_nr(float *a, long nrl, long nrh, long ncl, long nch);
float *convert_nr_to_matrix(float **mnr, long w, long h);
void free_nr_matrix(float **m, long nrl, long nrh, long ncl, long nch);
void gaussj(float **a, int n, float **b, int m);

#endif
