#include "common/defs.h"
#include "common/win_names.h"
#include "common/globs.h"
#include "utils/utils.h"
#include "warptrap/wtrap.h"
#include "gui/handlers.h"
#include "gui/windraw.h"

void init_wts(Uint32 wtn);

// ************* // 

int main(int argc, char *argv[]) {
	Uint32 nwidth, nheight;
	Uint32 i;

	Uint8 autoWarp = 0; // Auto warp learning

	// Check parameters and load image file
	if (argc < 2) {
		fprintf(stdout, "%s imagefile [-a]\n", argv[0]);
		return 1;
	}

	if (!(oimg = cvLoadImage(argv[1], CV_LOAD_IMAGE_UNCHANGED))) {
		fprintf(stderr, "Unable to load image %s\n", argv[1]);
		return 1;
	} else {
		fprintf(stdout, "Loaded image %s\n", argv[1]);
	}

	if (argc > 2) {
		if (strcmp(argv[2], "-a") == 0) autoWarp = 1; // try to automatize warp windows positions
	}

	// This is probably a shot of a single page, only one wts needed
	if (oimg->height > oimg->width) 
		used_wts = 1;

	// Prepare file path, removing last 4 chars (extension)
	strncat(dest_file, argv[1], strlen(argv[1]) - 4);

	// Load preview win names
	strncpy(win_str, PREV_WIN_1, 64);

	// Initialize the warptraps
	init_wts(MAX_WTS);

	// Prepare the memory for warp matrices
	for (i = 0; i < MAX_WTS; i++) {
		wtcode[i] = i;

		if (i < used_wts)
			invt[i] = cvCreateMat(3, 3, CV_32FC1);
		else
			invt[i] = NULL;
	}

	IplImage *tmp_img;
	nwidth = oimg->width;
	nheight = oimg->height;

	// Prepare original image to work on (size preview * 4)
	recalc_img_size(&nwidth, &nheight, PREV_H * 4);
	tmp_img = cvCreateImage(cvSize(nwidth , nheight), oimg->depth, oimg->nChannels); // Create a resized image
	cvResize(oimg, tmp_img, CV_INTER_CUBIC); // Resize

	// Save as new original image
	cvReleaseImage(&oimg);
	oimg = tmp_img;

	nwidth = oimg->width;
	nheight = oimg->height;

	// Prepare preview image
	recalc_img_size(&nwidth, &nheight, PREV_H);
	mw_img = cvCreateImage(cvSize(nwidth , nheight), oimg->depth, oimg->nChannels); // Create a resized image
	cvResize(oimg, mw_img, CV_INTER_LINEAR); // Resize


	// Create images for preview
	for (i = 0; i < MAX_WTS; i++) {
		if (i < used_wts)
			prv_img[i] = cvCreateImage(cvSize(PREV_W, PREV_H), oimg->depth, oimg->nChannels);
		else
			prv_img[i] = NULL;
	}

	// Create windows
	cvNamedWindow(MAIN_WIN, CV_WINDOW_AUTOSIZE);

	// Create and move preview windows
	for (i = 0; i < used_wts; i++) {
		win_str[19] = 49 + i;
		cvNamedWindow(win_str, CV_WINDOW_AUTOSIZE);
		cvMoveWindow(win_str, (18 + PREV_W) *(i + 1), 10);
	}

	// Build control window
	cvNamedWindow(CNTRL_WIN, CV_WINDOW_NORMAL);

	// Move main and control window
	cvMoveWindow(MAIN_WIN, 10, 10);
	cvMoveWindow(CNTRL_WIN, 40 + PREV_W * 2, 50);

	// Prepare handlers for trackbars
	int bgr_trkval = 1;
	int msk_trkval = (DEFAULT_TMASK - 1) / 20;
	int avr_trkval = DEFAULT_RMTH;
	cvCreateTrackbar(PREV_TRK_BGR, CNTRL_WIN, &bgr_trkval, 2, cntrl_trk_bgr_handler);
	cvCreateTrackbar(PREV_TRK_MSK, CNTRL_WIN, &msk_trkval, 30, cntrl_trk_tmask_handler);
	cvCreateTrackbar(PREV_TRK_AVR, CNTRL_WIN, &avr_trkval, 255, cntrl_trk_avr_handler);

	// Build transform matrices
	for (i = 0; i < used_wts; i++) {
		win_str[19] = 49 + i;
		invt[i] = build_transf_mat(&wt[i], invt[i], oimg, mw_img, prv_img[i]->width, prv_img[i]->height);
		redraw_preview_win(prv_img[i], win_str, oimg, invt[i], &wt[i]);
	}

	// Draw main window with warptraps
	draw_wt_win(MAIN_WIN, mw_img, wt, used_wts);

	// Register mouse handler for main window
	cvSetMouseCallback(MAIN_WIN, main_mouseHandler, (void *)mw_img);

	// Register mouse handlers for previews
	for (i = 0; i < used_wts; i++) {
		win_str[19] = 49 + i;
		cvSetMouseCallback(win_str, prev_mouseHandler, &wtcode[i]);
	}

	// wait for a key
	while(cvWaitKey(0) != 'q');

	// Destroy windows
	cvDestroyAllWindows();

	// Release memory
	cvReleaseImage(&oimg);
	cvReleaseImage(&mw_img);
	for (i = 0; i < used_wts; i++) {
		cvReleaseImage(&prv_img[i]);
		cvReleaseMat(&invt[i]);
	}

	return 0;
}

void init_wts(Uint32 wtn) {
	Uint16 i;

	Sint32 x = 40;
	Sint32 y = 20;

	for (i = 0; i < wtn; i++) {
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

