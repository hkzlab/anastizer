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

void update_wt_win(char *win, IplImage *in, WTrap *wt, CvScalar s) {
	IplImage *cin = cvCloneImage(in);

	draw_wtrap(cin, wt, s);
	cvShowImage(win, cin);

	cvReleaseImage(&cin);
}
