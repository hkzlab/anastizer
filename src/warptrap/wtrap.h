#ifndef _WTRAP_HEADER_
#define _WTRAP_HEADER_

#include "common/defs.h"

typedef struct {
	CvPoint a, b, c, d;
} WTrap;

void draw_wtrap(IplImage *in, WTrap *wt, CvScalar s);
void check_wtrap_point(Sint32 x, Sint32 y, WTrap *wt, Sint32 *node, Uint32 tot_wt);

#endif /* _WTRAP_HEADER_ */
