#ifndef _HANDLERS_HEADER_
#define _HANDLERS_HEADER_

#include "common/defs.h"
#include "common/globs.h"

void main_mouseHandler(int event, int x, int y, int flags, void *param);
void prev_mouseHandler(int event, int x, int y, int flags, void *param);
void cntrl_trk_bgr_handler(int pos);
void cntrl_trk_tmask_handler(int pos);
void cntrl_trk_avr_handler(int pos);
void redraw_preview_win(IplImage *pim, const char *win, IplImage *oim, CvMat *tm, WTrap *wt);

#endif /* _HANDLERS_HEADER_ */
