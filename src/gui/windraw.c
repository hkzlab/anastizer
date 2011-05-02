#include "windraw.h"

#include "common/win_names.h"
#include "utils/utils.h"

void redraw_preview_win(IplImage *pim, const char *win, IplImage *oim, CvMat *tm, WTrap *wt) {
	assert(pim);
	assert(oim);
	assert(tm);

	cvWarpPerspective(oim, pim, tm, /*CV_INTER_LINEAR +*/ CV_WARP_FILL_OUTLIERS + CV_WARP_INVERSE_MAP, cvScalarAll(0));

	IplImage *mono = gray_from_colour(pim, cvGetTrackbarPos(PREV_TRK_BGR, CNTRL_WIN));

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

