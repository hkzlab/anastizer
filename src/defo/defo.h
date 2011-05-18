#ifndef _DEFO_HEADER_
#define _DEFO_HEADER_

#include "common/defs.h"

// Deformation grid
typedef struct {
	Sint32 width;
	Sint32 height;
	CvPoint2D32f *pnt;
} defo_grid;

// Identifies a single defor.
// point in the grid
typedef struct {
	Sint32 px;
	Sint32 py;
	CvPoint2D32f *pnt;
} defo_point;

defo_grid *allocDefoGrid(Sint32 width, Sint32 height);
void freeDefoGrid(defo_grid **dg);
void initDefoGrid(IplImage *img, defo_grid *grid);
defo_point *findDefoPoint(Sint32 xclick, Sint32 yclick, defo_grid *grid);
void moveDefoPoint(Sint32 xdiff, Sint32 ydiff, defo_point *dp, defo_grid *grid);
void drawDefoGrid(IplImage *img, defo_grid *grid, CvScalar col);
IplImage *warpDefoImg(IplImage *img, defo_grid *dgrid, defo_grid *ogrid);

#endif /* _DEFO_HEADER_ */
