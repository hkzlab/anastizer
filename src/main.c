#include "common/defs.h"
#include "common/win_names.h"
#include "utils/utils.h"
#include "warptrap/wtrap.h"
#include "spotclear/spotclear.h"

#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_RMTH 5
#define DEFAULT_TMASK 301

#define STRSIZE 256
static char dest_file[STRSIZE];

#define TOT_WTS 1
static WTrap wt[TOT_WTS];

#define PREV_H 512
#define PREV_W 352
CvMat *invt[TOT_WTS];

static IplImage *oimg; // Original image;
static IplImage *prv_img;
static IplImage *mw_img;

void init_wts(void);
void main_mouseHandler(int event, int x, int y, int flags, void *param);
void prev_mouseHandler(int event, int x, int y, int flags, void *param);
void prv_trk_bgr_handler(int pos);

void update_preview_win(IplImage *pim, IplImage *oim, CvMat* tm, WTrap *wt);

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

	// Calculate output filename (PNG format)
	strncat(dest_file, argv[1], strlen(argv[1]) - 4);
	strcat(dest_file, "_WARPED.png");

	init_wts();

	for (i = 0; i < TOT_WTS; i++)
		invt[i] = cvCreateMat(3, 3, CV_32FC1); // Allocate space for transf matrices

	nwidth = oimg->width;
	nheight = oimg->height;

	recalc_img_size(&nwidth, &nheight, 512);
	mw_img = cvCreateImage(cvSize(nwidth , nheight), oimg->depth, oimg->nChannels); // Create a resized image
	cvResize(oimg, mw_img, CV_INTER_NN); // Resize
	prv_img = cvCreateImage(cvSize(PREV_W, PREV_H), oimg->depth, oimg->nChannels);

	// Create main window
	cvNamedWindow(MAIN_WIN, CV_WINDOW_AUTOSIZE); 
	cvNamedWindow(PREV_WIN, CV_WINDOW_AUTOSIZE); 

	cvCreateTrackbar(PREV_TRK_BGR, PREV_WIN, NULL, 2, prv_trk_bgr_handler);

	for (i = 0; i < TOT_WTS; i++)
		invt[i] = build_transf_mat(&wt[i], invt[i], oimg, mw_img, prv_img->width, prv_img->height);

	update_preview_win(prv_img, oimg, invt[0], &wt[0]);

	// Show resized image
	update_wt_win(MAIN_WIN, mw_img, wt, cvScalar(0, 0, 255, 0));

	// Register mouse handler for main window
	cvSetMouseCallback(MAIN_WIN, main_mouseHandler, (void*)mw_img);
	cvSetMouseCallback(PREV_WIN, prev_mouseHandler, NULL);

	// wait for a key
  	cvWaitKey(0);

	// Destroy windows
	cvDestroyWindow(MAIN_WIN);
	cvDestroyWindow(PREV_WIN);

	cvReleaseImage(&oimg); // Release greyscale image
	cvReleaseImage(&mw_img); 
	cvReleaseImage(&prv_img);  

	for (i = 0; i < TOT_WTS; i++)
		cvReleaseMat(&invt[i]);

	return 0;
}

void prev_mouseHandler(int event, int x, int y, int flags, void *param) {
	IplImage *gimg;
	IplImage *mimg;

	int sres;
	Uint8 cur_chan;

	switch(event) {
		case CV_EVENT_LBUTTONDBLCLK:
			cur_chan = cvGetTrackbarPos(PREV_TRK_BGR, PREV_WIN);
			invt[0] = build_transf_mat(&wt[0], invt[0], oimg, mw_img, prv_img->width * 4, prv_img->height * 4);
			gimg = return_warped_img(oimg, invt[0], &wt[0], prv_img->width * 4, prv_img->height * 4, cur_chan);
			mimg = cvCreateImage(cvGetSize(gimg), 8, 1);
			
			fprintf(stdout, " Applying local thresholding to image...\n");
			cvAdaptiveThreshold(gimg, mimg, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, DEFAULT_TMASK, DEFAULT_RMTH);


			fprintf(stdout, " Applying spot cleanup based on size...\n");
			remove_spot_size(mimg, 30, Conn8); // Do a spot cleanup
			fprintf(stdout, " Applying spot cleanup based on intensity...\n");
			remove_spot_intensity(mimg, gimg, 500, -50, cur_chan, Conn4);
			fprintf(stdout, " Applying spot cleanup based on thinness...\n");
			spot_thin(mimg, 50, 0.5, Conn8);
			fprintf(stdout, " Applying spot cleanup based on distance...\n");
			spot_neighbour_dist(mimg, 50, 20, Conn8);
			spot_neighbour_dist(mimg, 400, 45, Conn8);

			// Save it...
			fprintf(stdout, "saving to %s ...", dest_file);
			sres = cvSaveImage(dest_file, mimg, 0);

			if (sres)
				fprintf(stdout, "OK!\n\n");
			else
				fprintf(stdout, "Not saved!!!\n\n");

			cvReleaseImage(&gimg);
			cvReleaseImage(&mimg);

			break;
	}

	return;
}

void main_mouseHandler(int event, int x, int y, int flags, void *param) {
	static Uint8 lb_down = 0;
	static Uint8 rb_down = 0;
	static Sint32 curnode = -1;

	static int oldx, oldy;
	int xdiff, ydiff;

	switch(event) {
		case CV_EVENT_LBUTTONDOWN:
		case CV_EVENT_RBUTTONDOWN:
			check_wtrap_point(x, y, &wt[0], &curnode);
			if (event == CV_EVENT_LBUTTONDOWN) lb_down = 1;
			else if (event == CV_EVENT_RBUTTONDOWN) rb_down = 1;
			oldx = x;
			oldy = y;
			break;
		case CV_EVENT_RBUTTONUP:
			rb_down = 0;
			if (curnode >= 0) { // And in this case we should update a preview window...
				invt[0] = build_transf_mat(&wt[0], invt[0], oimg, mw_img, prv_img->width, prv_img->height);
				update_preview_win(prv_img, oimg, invt[0], &wt[0]);
			}

			curnode = -1;
			break;
		case CV_EVENT_LBUTTONUP:
			lb_down = 0;
			if (curnode >= 0) { // And in this case we should update a preview window...
				invt[0] = build_transf_mat(&wt[0], invt[0], oimg, mw_img, prv_img->width, prv_img->height);
				update_preview_win(prv_img, oimg, invt[0], &wt[0]);
			}
			
			curnode = -1;
			break;
		case CV_EVENT_MOUSEMOVE:
			if (curnode < 0 || (!lb_down && !rb_down)) break;

			xdiff = x - oldx;
			ydiff = y - oldy;
	
			if (lb_down) {
				if (curnode == 0) {
					wt[0].a.x += xdiff;
					wt[0].a.y += ydiff;
	
					if(wt[0].a.x > wt[0].b.x - 10)
						wt[0].a.x = wt[0].b.x - 10;

					if(wt[0].a.x > wt[0].c.x - 10)
						wt[0].a.x = wt[0].c.x - 10;

					if(wt[0].a.y > wt[0].d.y - 10)
						wt[0].a.y = wt[0].d.y - 10;

					if(wt[0].a.y > wt[0].c.y - 10)
						wt[0].a.y = wt[0].c.y - 10;

				} else if (curnode == 1) {
					wt[0].b.x += xdiff;
					wt[0].b.y += ydiff;

					if(wt[0].a.x + 10 > wt[0].b.x)
						wt[0].b.x = wt[0].a.x + 10;

					if(wt[0].d.x + 10 > wt[0].b.x)
						wt[0].b.x = wt[0].d.x + 10;
				
					if(wt[0].b.y > wt[0].c.y - 10)
						wt[0].b.y = wt[0].c.y - 10;

					if(wt[0].b.y > wt[0].d.y - 10)
						wt[0].b.y = wt[0].d.y - 10;

				} else if (curnode == 2) {
					wt[0].c.x += xdiff;
					wt[0].c.y += ydiff;

					if(wt[0].d.x + 10 > wt[0].c.x)
						wt[0].c.x = wt[0].d.x + 10;

					if(wt[0].a.x + 10 > wt[0].c.x)
						wt[0].c.x = wt[0].a.x + 10;

					if(wt[0].b.y + 10 > wt[0].c.y)
						wt[0].c.y = wt[0].b.y + 10;

					if(wt[0].a.y + 10 > wt[0].c.y)
						wt[0].c.y = wt[0].a.y + 10;

				} else if (curnode == 3) {
					wt[0].d.x += xdiff;
					wt[0].d.y += ydiff;

					if(wt[0].d.x > wt[0].c.x - 10)
						wt[0].d.x = wt[0].c.x - 10;

					if(wt[0].d.x > wt[0].b.x - 10)
						wt[0].d.x = wt[0].b.x - 10;

					if(wt[0].a.y + 10 > wt[0].d.y)
						wt[0].d.y = wt[0].a.y + 10;

					if(wt[0].b.y + 10 > wt[0].d.y)
						wt[0].d.y = wt[0].b.y + 10;
				}
			} else if (rb_down) {
				if (wt[0].a.y + ydiff >= 0 && wt[0].a.x + xdiff >= 0 && \
				wt[0].b.y + ydiff >= 0 && wt[0].b.x + xdiff < ((IplImage*)param)->width && \
				wt[0].c.y + ydiff < ((IplImage*)param)->height && wt[0].c.x + xdiff < ((IplImage*)param)->width && \
				wt[0].d.y + ydiff < ((IplImage*)param)->height && wt[0].d.x + xdiff >= 0) {
					
					wt[0].a.x += xdiff;
					wt[0].b.x += xdiff;
					wt[0].c.x += xdiff;
					wt[0].d.x += xdiff;

					wt[0].a.y += ydiff;
					wt[0].b.y += ydiff;
					wt[0].c.y += ydiff;
					wt[0].d.y += ydiff;
				}

			}

			oldx = x;
			oldy = y;

			update_wt_win(MAIN_WIN, (IplImage*)param, wt, cvScalar(0, 0, 255, 0));
			
#ifndef _SLOW_SYSTEM_
			invt[0] = build_transf_mat(&wt[0], invt[0], oimg, mw_img, prv_img->width, prv_img->height);
			update_preview_win(prv_img, oimg, invt[0], &wt[0]);
#endif

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
		wt[i].a.x = x;
		wt[i].a.y = y;

		wt[i].b.x = x + 40;
		wt[i].b.y = y;

		wt[i].c.x = x + 40;
		wt[i].c.y = y + 40;

		wt[i].d.x = x;
		wt[i].d.y = y + 40;
	}
}

void update_preview_win(IplImage *pim, IplImage *oim, CvMat* tm, WTrap *wt) {
	assert(pim);
	assert(oim);
	assert(tm);

	cvWarpPerspective(oim, pim, tm, /*CV_INTER_LINEAR +*/ CV_WARP_FILL_OUTLIERS + CV_WARP_INVERSE_MAP, cvScalarAll(0));

	IplImage *mono = gray_from_colour(pim, cvGetTrackbarPos(PREV_TRK_BGR, PREV_WIN));

	cvShowImage(PREV_WIN, mono);

	cvReleaseImage(&mono);
}

void prv_trk_bgr_handler(int pos) {
	update_preview_win(prv_img, oimg, invt[0], &wt[0]);
}

