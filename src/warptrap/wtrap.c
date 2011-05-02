#include "wtrap.h"

void draw_wtrap(IplImage *in, WTrap *wt, CvScalar s) {
	assert(in);
	assert(wt);

	cvLine(in, wt->a, wt->b, s, 1, 8, 0);
	cvLine(in, wt->b, wt->c, s, 1, 8, 0);
	cvLine(in, wt->c, wt->d, s, 1, 8, 0);
	cvLine(in, wt->d, wt->a, s, 1, 8, 0);
	cvCircle(in, wt->a, 4, s, 2, 8, 0);
	cvCircle(in, wt->b, 4, s, 2, 8, 0);
	cvCircle(in, wt->c, 4, s, 2, 8, 0);
	cvCircle(in, wt->d, 4, s, 2, 8, 0);
}

void check_wtrap_point(Sint32 x, Sint32 y, WTrap *wt, Sint32 *node, Uint32 tot_wt) {
	Uint32 i;
	*node = -1;

	for (i = 0; i < tot_wt; i++) {
		if (abs(wt[i].a.x - x) <= 5 && abs(wt[i].a.y - y) <= 5) *node = 0 + 4 * i;
		else if (abs(wt[i].b.x - x) <= 5 && abs(wt[i].b.y - y) <= 5) *node = 1 + 4 * i;
		else if (abs(wt[i].c.x - x) <= 5 && abs(wt[i].c.y - y) <= 5) *node = 2 + 4 * i;
		else if (abs(wt[i].d.x - x) <= 5 && abs(wt[i].d.y - y) <= 5) *node = 3 + 4 * i;
	}

}
