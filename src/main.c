#include "common/defs.h"
#include "common/win_names.h"
#include "utils/utils.h"
#include "warptrap/wtrap.h"
#include "spotclear/spotclear.h"

#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_RMTH 5
#define DEFAULT_TMASK 301

static Uint16 tmask_size = DEFAULT_TMASK;
static Uint16 tmask_avr = DEFAULT_RMTH;

#define STRSIZE 256
static char dest_file[STRSIZE];

#define TOT_WTS 2
static WTrap wt[TOT_WTS];

#define PREV_H 512
#define PREV_W 351
CvMat *invt[TOT_WTS];

static IplImage *oimg; // Original image;
static IplImage *prv_img[TOT_WTS];
static IplImage *mw_img;
static char win_str[64];

void init_wts(void);
void main_mouseHandler(int event, int x, int y, int flags, void *param);
void prev_mouseHandler(int event, int x, int y, int flags, void *param);
void cntrl_trk_bgr_handler(int pos);
void cntrl_trk_tmask_handler(int pos);
void cntrl_trk_avr_handler(int pos);

void update_preview_win(IplImage *pim, const char *win, IplImage *oim, CvMat *tm, WTrap *wt);

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

	for (i = 0; i < TOT_WTS; i++)
		invt[i] = cvCreateMat(3, 3, CV_32FC1); // Allocate space for transf matrices

	nwidth = oimg->width;
	nheight = oimg->height;

	recalc_img_size(&nwidth, &nheight, PREV_H);
	mw_img = cvCreateImage(cvSize(nwidth , nheight), oimg->depth, oimg->nChannels); // Create a resized image
	cvResize(oimg, mw_img, CV_INTER_LINEAR); // Resize

	for (i = 0; i < TOT_WTS; i++)
		prv_img[i] = cvCreateImage(cvSize(PREV_W, PREV_H), oimg->depth, oimg->nChannels);

	// Create windows
	cvNamedWindow(MAIN_WIN, CV_WINDOW_NORMAL);

	
	for (i = 0; i < TOT_WTS; i++) {
		win_str[19] = 49 + i;
		cvNamedWindow(win_str, CV_WINDOW_NORMAL);
		cvResizeWindow(win_str, PREV_W, PREV_H);
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

	for (i = 0; i < TOT_WTS; i++) {
		win_str[19] = 49 + i;
		invt[i] = build_transf_mat(&wt[i], invt[i], oimg, mw_img, prv_img[i]->width, prv_img[i]->height);
		update_preview_win(prv_img[i], win_str, oimg, invt[i], &wt[i]);
	}

	update_wt_win(MAIN_WIN, mw_img, wt, TOT_WTS);

	// Register mouse handler for main window
	cvSetMouseCallback(MAIN_WIN, main_mouseHandler, (void *)mw_img);
	//cvSetMouseCallback(PREV_WIN, prev_mouseHandler, NULL);

	// wait for a key
	cvWaitKey(0);

	// Destroy windows
	cvDestroyWindow(MAIN_WIN);

	for (i = 0; i < TOT_WTS; i++) {
		win_str[19] = 49 + i;
		cvDestroyWindow(win_str);
	}

	cvDestroyWindow(CNTRL_WIN);
//	cvDestroyAllWindows();

	cvReleaseImage(&oimg); // Release greyscale image
	cvReleaseImage(&mw_img);
	
	for (i = 0; i < TOT_WTS; i++) {
		cvReleaseImage(&prv_img[i]);
		cvReleaseMat(&invt[i]);
	}

	return 0;
}

#if 0
void prev_mouseHandler(int event, int x, int y, int flags, void *param) {
	IplImage *gimg;
	IplImage *mimg;
	IplImage *rprev;

	int sres;
	Uint8 cur_chan;
	Uint32 nwidth, nheight;

	char tmp_file[STRSIZE];
	tmp_file[0] = '\0';

	switch (event) {
	case CV_EVENT_MBUTTONDBLCLK:
	case CV_EVENT_LBUTTONDBLCLK:
	case CV_EVENT_RBUTTONDBLCLK:

		strcat(tmp_file, dest_file);

		cur_chan = cvGetTrackbarPos(PREV_TRK_BGR, CNTRL_WIN);
		invt[0] = build_transf_mat(&wt[0], invt[0], oimg, mw_img, prv_img->width * 4, prv_img->height * 4);
		
		if (event == CV_EVENT_MBUTTONDBLCLK) { // Save a color version
			gimg = return_warped_img(oimg, invt[0], &wt[0], prv_img->width * 4, prv_img->height * 4, -1);

			// Calculate output filename (JPG format)
			strcat(tmp_file, "_WARPED.jpg");
			
			fprintf(stdout, " saving [%ux%u] image to %s ...", gimg->width, gimg->height, tmp_file);
			sres = cvSaveImage(tmp_file, gimg, 0);

			if (sres)
				fprintf(stdout, "OK!\n");
			else
				fprintf(stdout, "Not saved!!!\n");

			fprintf(stdout, " DONE!\n\n");

			cvReleaseImage(&gimg);
			break;

		} else {
			gimg = return_warped_img(oimg, invt[0], &wt[0], prv_img->width * 4, prv_img->height * 4, cur_chan);
		}

		mimg = cvCreateImage(cvGetSize(gimg), 8, 1);

		fprintf(stdout, " Applying local thresholding to image...\n");
		cvAdaptiveThreshold(gimg, mimg, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, tmask_size, tmask_avr);


		fprintf(stdout, " Applying spot cleanup based on size...\n");
		remove_spot_size(mimg, 10, Conn8); // Do a spot cleanup
		fprintf(stdout, " Applying spot cleanup based on intensity...\n");
		remove_spot_intensity(mimg, gimg, 500, -50, cur_chan, Conn4);
		fprintf(stdout, " Applying spot cleanup based on thinness...\n");
		spot_thin(mimg, 50, 0.5, Conn8);
		fprintf(stdout, " Applying spot cleanup based on distance...\n");
		spot_neighbour_dist(mimg, 50, 20, Conn8);
		spot_neighbour_dist(mimg, 400, 45, Conn8);

		// Show it...
		nwidth = mimg->width;
		nheight = mimg->height;
		recalc_img_size(&nwidth, &nheight, PREV_H);
		rprev = cvCreateImage(cvSize(nwidth , nheight), mimg->depth, mimg->nChannels); // Create a resized image
		cvResize(mimg, rprev, CV_INTER_CUBIC); // Resize

		cvShowImage(PREV_WIN, rprev);

		if (event == CV_EVENT_RBUTTONDBLCLK) { // or Save it...
			// Calculate output filename (PNG format)
			strcat(tmp_file, "_ANAST.png");

			fprintf(stdout, " saving [%ux%u] image to %s ...", mimg->width, mimg->height, tmp_file);
			sres = cvSaveImage(tmp_file, mimg, 0);

			if (sres)
				fprintf(stdout, "OK!\n");
			else
				fprintf(stdout, "Not saved!!!\n");
		}

		cvReleaseImage(&gimg);
		cvReleaseImage(&rprev);
		cvReleaseImage(&mimg);

		fprintf(stdout, " DONE!\n\n");

		break;
	}

	return;
}
#endif

void main_mouseHandler(int event, int x, int y, int flags, void *param) {
	static Uint8 lb_down = 0;
	static Uint8 rb_down = 0;
	static Uint8 mb_down = 0;
	static Sint32 curnode = -1;
	static Sint32 cur_wts = -1;

	static int oldx, oldy;
	int xdiff, ydiff;

	Uint32 i;

	switch (event) {
	case CV_EVENT_LBUTTONDOWN:
	case CV_EVENT_RBUTTONDOWN:
	case CV_EVENT_MBUTTONDOWN:
		check_wtrap_point(x, y, wt, &curnode, TOT_WTS);

		cur_wts = curnode / 4;
		curnode %= 4;

		if (event == CV_EVENT_LBUTTONDOWN) lb_down = 1;
		else if (event == CV_EVENT_RBUTTONDOWN) rb_down = 1;
		else if (event == CV_EVENT_MBUTTONDOWN) mb_down = 1;
		oldx = x;
		oldy = y;
		break;
	case CV_EVENT_MBUTTONUP:
	case CV_EVENT_RBUTTONUP:
	case CV_EVENT_LBUTTONUP:
		if (event == CV_EVENT_LBUTTONUP) lb_down = 0;
		else if (event == CV_EVENT_RBUTTONUP) rb_down = 0;
		else if (event == CV_EVENT_MBUTTONUP) mb_down = 0;

		if (curnode >= 0) { // And in this case we should update a preview window...
			//invt[0] = build_transf_mat(&wt[0], invt[0], oimg, mw_img, prv_img->width, prv_img->height);
			//update_preview_win(prv_img, oimg, invt[0], &wt[0]);
		}

		curnode = -1;
		break;
	case CV_EVENT_MOUSEMOVE:
		if (curnode < 0 || (!lb_down && !rb_down && !mb_down)) break;

		xdiff = x - oldx;
		ydiff = y - oldy;

		if (lb_down) {
			if (curnode == 0) {
				wt[cur_wts].a.x += xdiff;
				wt[cur_wts].a.y += ydiff;

				if (wt[cur_wts].a.x > wt[cur_wts].b.x - 10)
					wt[cur_wts].a.x = wt[cur_wts].b.x - 10;

				if (wt[cur_wts].a.x > wt[cur_wts].c.x - 10)
					wt[cur_wts].a.x = wt[cur_wts].c.x - 10;

				if (wt[cur_wts].a.y > wt[cur_wts].d.y - 10)
					wt[cur_wts].a.y = wt[cur_wts].d.y - 10;

				if (wt[cur_wts].a.y > wt[cur_wts].c.y - 10)
					wt[cur_wts].a.y = wt[cur_wts].c.y - 10;

			} else if (curnode == 1) {
				wt[cur_wts].b.x += xdiff;
				wt[cur_wts].b.y += ydiff;

				if (wt[cur_wts].a.x + 10 > wt[cur_wts].b.x)
					wt[cur_wts].b.x = wt[cur_wts].a.x + 10;

				if (wt[cur_wts].d.x + 10 > wt[cur_wts].b.x)
					wt[cur_wts].b.x = wt[cur_wts].d.x + 10;

				if (wt[cur_wts].b.y > wt[cur_wts].c.y - 10)
					wt[cur_wts].b.y = wt[cur_wts].c.y - 10;

				if (wt[cur_wts].b.y > wt[cur_wts].d.y - 10)
					wt[cur_wts].b.y = wt[cur_wts].d.y - 10;

			} else if (curnode == 2) {
				wt[cur_wts].c.x += xdiff;
				wt[cur_wts].c.y += ydiff;

				if (wt[cur_wts].d.x + 10 > wt[cur_wts].c.x)
					wt[cur_wts].c.x = wt[cur_wts].d.x + 10;

				if (wt[cur_wts].a.x + 10 > wt[cur_wts].c.x)
					wt[cur_wts].c.x = wt[cur_wts].a.x + 10;

				if (wt[cur_wts].b.y + 10 > wt[cur_wts].c.y)
					wt[cur_wts].c.y = wt[cur_wts].b.y + 10;

				if (wt[cur_wts].a.y + 10 > wt[cur_wts].c.y)
					wt[cur_wts].c.y = wt[cur_wts].a.y + 10;

			} else if (curnode == 3) {
				wt[cur_wts].d.x += xdiff;
				wt[cur_wts].d.y += ydiff;

				if (wt[cur_wts].d.x > wt[cur_wts].c.x - 10)
					wt[cur_wts].d.x = wt[cur_wts].c.x - 10;

				if (wt[cur_wts].d.x > wt[cur_wts].b.x - 10)
					wt[cur_wts].d.x = wt[cur_wts].b.x - 10;

				if (wt[cur_wts].a.y + 10 > wt[cur_wts].d.y)
					wt[cur_wts].d.y = wt[cur_wts].a.y + 10;

				if (wt[cur_wts].b.y + 10 > wt[cur_wts].d.y)
					wt[cur_wts].d.y = wt[cur_wts].b.y + 10;
			}
		} else if (rb_down) {
			if (wt[cur_wts].a.y + ydiff >= 0 && wt[cur_wts].a.x + xdiff >= 0 && \
			        wt[cur_wts].b.y + ydiff >= 0 && wt[cur_wts].b.x + xdiff < ((IplImage *)param)->width && \
			        wt[cur_wts].c.y + ydiff < ((IplImage *)param)->height && wt[cur_wts].c.x + xdiff < ((IplImage *)param)->width && \
			        wt[cur_wts].d.y + ydiff < ((IplImage *)param)->height && wt[cur_wts].d.x + xdiff >= 0) {

				wt[cur_wts].a.x += xdiff;
				wt[cur_wts].b.x += xdiff;
				wt[cur_wts].c.x += xdiff;
				wt[cur_wts].d.x += xdiff;

				wt[cur_wts].a.y += ydiff;
				wt[cur_wts].b.y += ydiff;
				wt[cur_wts].c.y += ydiff;
				wt[cur_wts].d.y += ydiff;
			}

		} else if (mb_down) {
			if (wt[cur_wts].a.y + ydiff >= 0 && wt[cur_wts].a.x + xdiff >= 0 && \
			        wt[cur_wts].b.y + ydiff >= 0 && wt[cur_wts].b.x + xdiff < ((IplImage *)param)->width && \
			        wt[cur_wts].c.y + ydiff < ((IplImage *)param)->height && wt[cur_wts].c.x + xdiff < ((IplImage *)param)->width && \
			        wt[cur_wts].d.y + ydiff < ((IplImage *)param)->height && wt[cur_wts].d.x + xdiff >= 0) {
				if (curnode == 0) {
					wt[cur_wts].a.x += xdiff;
					wt[cur_wts].a.y += ydiff;

					wt[cur_wts].b.y += ydiff;
					wt[cur_wts].d.x += xdiff;

					if (wt[cur_wts].a.x >= wt[cur_wts].b.x - 10) wt[cur_wts].a.x = wt[cur_wts].b.x - 10;
					if (wt[cur_wts].d.x >= wt[cur_wts].c.x - 10) wt[cur_wts].d.x = wt[cur_wts].c.x - 10;
					if (wt[cur_wts].a.y >= wt[cur_wts].d.y - 10) wt[cur_wts].a.y = wt[cur_wts].d.y - 10;
					if (wt[cur_wts].b.y >= wt[cur_wts].c.y - 10) wt[cur_wts].b.y = wt[cur_wts].c.y - 10;

				} else if (curnode == 1) {
					wt[cur_wts].b.x += xdiff;
					wt[cur_wts].b.y += ydiff;

					wt[cur_wts].a.y += ydiff;
					wt[cur_wts].c.x += xdiff;

					if (wt[cur_wts].a.x >= wt[cur_wts].b.x - 10) wt[cur_wts].b.x = wt[cur_wts].a.x + 10;
					if (wt[cur_wts].d.x >= wt[cur_wts].c.x - 10) wt[cur_wts].c.x = wt[cur_wts].d.x + 10;
					if (wt[cur_wts].a.y >= wt[cur_wts].d.y - 10) wt[cur_wts].a.y = wt[cur_wts].d.y - 10;
					if (wt[cur_wts].b.y >= wt[cur_wts].c.y - 10) wt[cur_wts].b.y = wt[cur_wts].c.y - 10;

				} else if (curnode == 2) {
					wt[cur_wts].c.x += xdiff;
					wt[cur_wts].c.y += ydiff;

					wt[cur_wts].d.y += ydiff;
					wt[cur_wts].b.x += xdiff;

					if (wt[cur_wts].a.x >= wt[cur_wts].b.x - 10) wt[cur_wts].b.x = wt[cur_wts].a.x + 10;
					if (wt[cur_wts].d.x >= wt[cur_wts].c.x - 10) wt[cur_wts].c.x = wt[cur_wts].d.x + 10;
					if (wt[cur_wts].a.y >= wt[cur_wts].d.y - 10) wt[cur_wts].d.y = wt[cur_wts].a.y + 10;
					if (wt[cur_wts].b.y >= wt[cur_wts].c.y - 10) wt[cur_wts].c.y = wt[cur_wts].b.y + 10;

				} else if (curnode == 3) {
					wt[cur_wts].d.x += xdiff;
					wt[cur_wts].d.y += ydiff;

					wt[cur_wts].c.y += ydiff;
					wt[cur_wts].a.x += xdiff;

					if (wt[cur_wts].a.x >= wt[cur_wts].b.x - 10) wt[cur_wts].a.x = wt[cur_wts].b.x - 10;
					if (wt[cur_wts].d.x >= wt[cur_wts].c.x - 10) wt[cur_wts].d.x = wt[cur_wts].c.x - 10;
					if (wt[cur_wts].a.y >= wt[cur_wts].d.y - 10) wt[cur_wts].d.y = wt[cur_wts].a.y + 10;
					if (wt[cur_wts].b.y >= wt[cur_wts].c.y - 10) wt[cur_wts].c.y = wt[cur_wts].b.y + 10;

				}

			}
		}

		oldx = x;
		oldy = y;

		update_wt_win(MAIN_WIN, mw_img, wt, TOT_WTS);

		break;
	default:
		break;
	}

	return;
}

void init_wts(void) {
	Uint16 i;

	Sint32 x = 40;
	Sint32 y = 20;

	for (i = 0; i < TOT_WTS; i++) {
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

void update_preview_win(IplImage *pim, const char *win, IplImage *oim, CvMat *tm, WTrap *wt) {
	assert(pim);
	assert(oim);
	assert(tm);

	cvWarpPerspective(oim, pim, tm, /*CV_INTER_LINEAR +*/ CV_WARP_FILL_OUTLIERS + CV_WARP_INVERSE_MAP, cvScalarAll(0));

	IplImage *mono = gray_from_colour(pim, cvGetTrackbarPos(PREV_TRK_BGR, CNTRL_WIN));

	cvShowImage(win, mono);

	cvReleaseImage(&mono);
}

void cntrl_trk_bgr_handler(int pos) {
	Uint32 i;

	for (i = 0; i < TOT_WTS; i++) {
		win_str[19] = 49 + i;
		invt[i] = build_transf_mat(&wt[i], invt[i], oimg, mw_img, prv_img[i]->width, prv_img[i]->height);
		update_preview_win(prv_img[i], win_str, oimg, invt[i], &wt[i]);
	}
}

void cntrl_trk_tmask_handler(int pos) {
	tmask_size = ((pos + 1) * 20) + 1;
}

void cntrl_trk_avr_handler(int pos) {
	tmask_avr = pos;
}
