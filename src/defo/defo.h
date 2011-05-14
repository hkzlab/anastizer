#ifndef _DEFO_HEADER_
#define _DEFO_HEADER_

#include "common/defs.h"

typedef struct {
	Uint32 width;
	Uint32 height;
	CvPoint2D32f *pnt;
} defo_grid;

defo_grid *allocDefoGrid(Uint32 width, Uint32 height);
void *freeDefoGrid(defo_grid **dg);

#endif /* _DEFO_HEADER_ */
