#ifndef _WTRAP_HEADER_
#define _WTRAP_HEADER_

#include "common/defs.h"

typedef struct {
	CvPoint a, b, c, d;
} WTrap;

void draw_wtrap(IplImage *in, WTrap *wt, CvScalar s);
void update_wt_win(char *win, IplImage *in, WTrap *wt, CvScalar s);

#endif /* _WTRAP_HEADER_ */
