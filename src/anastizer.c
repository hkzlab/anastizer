#include "common/defs.h"
#include "common/anast_defs.h"
#include "common/win_names_anastizer.h"
#include "common/globs_anastizer.h"
#include "utils/utils.h"
#include "warptrap/wtrap.h"
#include "gui/handlers_anastizer.h"
#include "gui/windraw_anastizer.h"
#include "imc/imc.h"

void init_wts(Uint32 wtn);

// ************* //

int main(int argc, char *argv[]) {
	Uint32 nwidth, nheight;
	Uint32 i;
	float oxratio, oyratio; // Original ratios of image, which gets resized at startup
	float pxratio, pyratio;

	// Vars needed for loading/saving
	imc_data *dt = NULL;
	int saveload_res;
	char imc_file[STR_BUF_SIZE];

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
	recalc_img_size(&nwidth, &nheight, ORIG_H);
	tmp_img = cvCreateImage(cvSize(nwidth , nheight), oimg->depth, oimg->nChannels); // Create a resized image
	cvResize(oimg, tmp_img, CV_INTER_CUBIC); // Resize

	// Save the ratios
	oxratio = (float)oimg->width / (float)tmp_img->width;
	oyratio = (float)oimg->height / (float)tmp_img->height;

	// Save as new original image
	cvReleaseImage(&oimg);
	oimg = tmp_img;

	nwidth = oimg->width;
	nheight = oimg->height;

	// Prepare preview image
	recalc_img_size(&nwidth, &nheight, PREV_H);
	mw_img = cvCreateImage(cvSize(nwidth , nheight), oimg->depth, oimg->nChannels); // Create a resized image
	cvResize(oimg, mw_img, CV_INTER_LINEAR); // Resize

	// Save the ratios
	pxratio = (float)oimg->width / (float)mw_img->width;
	pyratio = (float)oimg->height / (float)mw_img->height;

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
	int avr_trkval = DEFAULT_RMTH;
	int qlt_trkval = WARP_MULT / 2 - 1;
	int msk_trkval = (DEFAULT_TMASK * (qlt_trkval + 1)) / 2;
	int agg_trkval = 5;
	cvCreateTrackbar(PREV_TRK_QLT, CNTRL_WIN, &qlt_trkval, 2, cntrl_trk_qlt_handler);
	cvCreateTrackbar(PREV_TRK_AGG, CNTRL_WIN, &agg_trkval, 9, cntrl_trk_agg_handler);
	cvCreateTrackbar(PREV_TRK_BGR, CNTRL_WIN, &bgr_trkval, 2, cntrl_trk_bgr_handler);
	cvCreateTrackbar(PREV_TRK_MSK, CNTRL_WIN, &msk_trkval, 1000, cntrl_trk_tmask_handler);
	cvCreateTrackbar(PREV_TRK_AVR, CNTRL_WIN, &avr_trkval, 255, cntrl_trk_avr_handler);

	// Load image config file, if present!
	imc_file[0] = '\0';
	strcat(imc_file, dest_file);
	strcat(imc_file, ".imc");
	dt = loadImcData(imc_file);
	if (dt) {
		if (dt->qlt_trk >= 0)
			cvSetTrackbarPos(PREV_TRK_QLT, CNTRL_WIN, dt->qlt_trk);
		
		if (dt->msk_trk >= 0)
			cvSetTrackbarPos(PREV_TRK_MSK, CNTRL_WIN, dt->msk_trk);
		
		if (dt->avr_trk >= 0)
			cvSetTrackbarPos(PREV_TRK_AVR, CNTRL_WIN, dt->avr_trk);
		
		if (dt->bgr_trk >= 0)
			cvSetTrackbarPos(PREV_TRK_BGR, CNTRL_WIN, dt->bgr_trk);

		if (dt->agg_trk >= 0)
			cvSetTrackbarPos(PREV_TRK_AGG, CNTRL_WIN, dt->agg_trk);

		for (i = 0; i < used_wts; i++) {
				wt[i].a.x = dt->wt[i].a.x / (oxratio * pxratio);
				wt[i].a.y = dt->wt[i].a.y / (oyratio * pyratio);
				wt[i].b.x = dt->wt[i].b.x / (oxratio * pxratio);
				wt[i].b.y = dt->wt[i].b.y / (oyratio * pyratio);
				wt[i].c.x = dt->wt[i].c.x / (oxratio * pxratio);
				wt[i].c.y = dt->wt[i].c.y / (oyratio * pyratio);
				wt[i].d.x = dt->wt[i].d.x / (oxratio * pxratio);
				wt[i].d.y = dt->wt[i].d.y / (oyratio * pyratio);
		}

		freeImcData(&dt);

		fprintf(stdout, "Successfully loaded image status file.\n");
	}

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

	// Manage keyboard input
	char key;
	while (1) {
		key = cvWaitKey(125);
		if (key == 'q') break;

		// Put some key management code in here
		switch (key) {
		case 's': // Save the data
			dt = allocImcData(used_wts);
			dt->qlt_trk = cvGetTrackbarPos(PREV_TRK_QLT, CNTRL_WIN);
			dt->msk_trk = cvGetTrackbarPos(PREV_TRK_MSK, CNTRL_WIN);
			dt->avr_trk = cvGetTrackbarPos(PREV_TRK_AVR, CNTRL_WIN);
			dt->bgr_trk = cvGetTrackbarPos(PREV_TRK_BGR, CNTRL_WIN);
			dt->agg_trk = cvGetTrackbarPos(PREV_TRK_AGG, CNTRL_WIN);

			for (i = 0; i < used_wts; i++) {
				// The coords gets saved in reference to the original, unscaled image
				dt->wt[i].a.x = wt[i].a.x * oxratio * pxratio;
				dt->wt[i].a.y = wt[i].a.y * oyratio * pyratio;
				dt->wt[i].b.x = wt[i].b.x * oxratio * pxratio;
				dt->wt[i].b.y = wt[i].b.y * oyratio * pyratio;
				dt->wt[i].c.x = wt[i].c.x * oxratio * pxratio;
				dt->wt[i].c.y = wt[i].c.y * oyratio * pyratio;
				dt->wt[i].d.x = wt[i].d.x * oxratio * pxratio;
				dt->wt[i].d.y = wt[i].d.y * oyratio * pyratio;
			}

			saveload_res = saveImcData(imc_file, dt);
			if (saveload_res >= 0) fprintf(stdout, "Successfully saved image status file.\n");
			else fprintf(stdout, "Unable to save image status file.\n");

			freeImcData(&dt);
			break;
		default:
			break;
		}
	}

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

