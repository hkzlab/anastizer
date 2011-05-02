#include "common/defs.h"
#include "common/win_names.h"
#include "common/globs.h"
#include "utils/utils.h"
#include "warptrap/wtrap.h"
#include "gui/handlers.h"
#include "gui/windraw.h"

void init_wts(void);


int main(int argc, char *argv[]) {
	Uint32 nwidth, nheight;
	Uint32 i;

	if (argc < 2) {
		fprintf(stdout, "%s [imagefile]\n", argv[0]);
		return 1;
	}

	if (!(oimg = cvLoadImage(argv[1], CV_LOAD_IMAGE_UNCHANGED))) {
		fprintf(stderr, "Unable to load image %s\n", argv[1]);
		return 1;
	} else {
		fprintf(stdout, "Loaded image %s\n", argv[1]);
	}

	// Load file path
	strncat(dest_file, argv[1], strlen(argv[1]) - 4);

	// Load preview win names
	strncpy(win_str, PREV_WIN_1, 64);

	init_wts();

	for (i = 0; i < MAX_WTS; i++) {
		wtcode[i] = i;
		invt[i] = cvCreateMat(3, 3, CV_32FC1); // Allocate space for transf matrices
	}

	nwidth = oimg->width;
	nheight = oimg->height;

	recalc_img_size(&nwidth, &nheight, PREV_H);
	mw_img = cvCreateImage(cvSize(nwidth , nheight), oimg->depth, oimg->nChannels); // Create a resized image
	cvResize(oimg, mw_img, CV_INTER_LINEAR); // Resize

	for (i = 0; i < MAX_WTS; i++)
		prv_img[i] = cvCreateImage(cvSize(PREV_W, PREV_H), oimg->depth, oimg->nChannels);

	// Create windows
	cvNamedWindow(MAIN_WIN, CV_WINDOW_AUTOSIZE);

	
	for (i = 0; i < MAX_WTS; i++) {
		win_str[19] = 49 + i;
		cvNamedWindow(win_str, CV_WINDOW_AUTOSIZE);
		//cvResizeWindow(win_str, PREV_W, PREV_H);
		cvMoveWindow(win_str, (18 + PREV_W) * (i + 1), 10);
	}
	
	cvNamedWindow(CNTRL_WIN, CV_WINDOW_NORMAL);

	// Resize windows
	cvResizeWindow(MAIN_WIN, PREV_W, PREV_H);

	// Move them
	cvMoveWindow(MAIN_WIN, 10, 10);
	cvMoveWindow(CNTRL_WIN, 40 + PREV_W * 2, 50);



	int bgr_trkval = 1;
	int msk_trkval = (DEFAULT_TMASK - 1)/20;
	int avr_trkval = DEFAULT_RMTH;
	cvCreateTrackbar(PREV_TRK_BGR, CNTRL_WIN, &bgr_trkval, 2, cntrl_trk_bgr_handler);
	cvCreateTrackbar(PREV_TRK_MSK, CNTRL_WIN, &msk_trkval, 30, cntrl_trk_tmask_handler);
	cvCreateTrackbar(PREV_TRK_AVR, CNTRL_WIN, &avr_trkval, 255, cntrl_trk_avr_handler);

	for (i = 0; i < MAX_WTS; i++) {
		win_str[19] = 49 + i;
		invt[i] = build_transf_mat(&wt[i], invt[i], oimg, mw_img, prv_img[i]->width, prv_img[i]->height);
		redraw_preview_win(prv_img[i], win_str, oimg, invt[i], &wt[i]);
	}

	draw_wt_win(MAIN_WIN, mw_img, wt, MAX_WTS);

	// Register mouse handler for main window
	cvSetMouseCallback(MAIN_WIN, main_mouseHandler, (void *)mw_img);

	for (i = 0; i < MAX_WTS; i++) {
		win_str[19] = 49 + i;
		cvSetMouseCallback(win_str, prev_mouseHandler, &wtcode[i]);
	}

	// wait for a key
	cvWaitKey(0);

	// Destroy windows
	cvDestroyAllWindows();

	cvReleaseImage(&oimg); // Release greyscale image
	cvReleaseImage(&mw_img);
	
	for (i = 0; i < MAX_WTS; i++) {
		cvReleaseImage(&prv_img[i]);
		cvReleaseMat(&invt[i]);
	}

	return 0;
}

void init_wts(void) {
	Uint16 i;

	Sint32 x = 40;
	Sint32 y = 20;

	for (i = 0; i < MAX_WTS; i++) {
		wt[i].a.x = x + (i * 140);
		wt[i].a.y = y;

		wt[i].b.x = wt[i].a.x + 120;
		wt[i].b.y = y;

		wt[i].c.x = wt[i].a.x + 120;
		wt[i].c.y = y + 210;

		wt[i].d.x = x + (i * 140);
		wt[i].d.y = y + 210;
	}
}

