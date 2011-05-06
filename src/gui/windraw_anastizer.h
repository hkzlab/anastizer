#ifndef _WINDRAW_ANASTIZER_HEADER_
#define _WINDRAW_ANASTIZER_HEADER_

#include "common/defs.h"
#include "warptrap/wtrap.h"

void redraw_preview_win(IplImage *pim, const char *win, IplImage *oim, CvMat *tm, WTrap *wt);
void draw_wt_win(char *win, IplImage *in, WTrap *wt, Uint32 tot_wt);

#endif /* _WINDRAW_ANASTIZER_HEADER_ */
