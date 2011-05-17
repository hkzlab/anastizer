#include "windraw_anastizer.h"

#include "common/win_names_anastizer.h"
#include "utils/utils.h"

#include "defo/defo.h"

void redraw_preview_win(IplImage *pim, const char *win, IplImage *oim, CvMat *tm, WTrap *wt, defo_grid *dgrid) {
	assert(pim);
	assert(oim);
	assert(tm);

	IplImage *mono;

	cvWarpPerspective(oim, pim, tm, /*CV_INTER_LINEAR +*/ CV_WARP_FILL_OUTLIERS + CV_WARP_INVERSE_MAP, cvScalarAll(0));

	Sint8 cur_chan = 0;
	cur_chan = cvGetTrackbarPos(PREV_TRK_BGR, CNTRL_WIN);
	mono = gray_from_colour(pim, cur_chan);

	if (dgrid) // Show the grid
		drawDefoGrid(mono, dgrid, cvScalarAll(0));

	cvShowImage(win, mono);

	cvReleaseImage(&mono);
}

void draw_wt_win(char *win, IplImage *in, WTrap *wt, Uint32 tot_wt) {
	Uint32 i;
	IplImage *cin = cvCloneImage(in);

	for (i = 0; i < tot_wt; i++)
		draw_wtrap(cin, &wt[i], cvScalar(((i + 1) * 60), 0, 255 - ((i + 1) * 40), 0));

	cvShowImage(win, cin);

	cvReleaseImage(&cin);
}

