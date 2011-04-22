#include "common/defs.h"
#include "common/win_names.h"
#include "gajor/gajor.h"
#include "utils/utils.h"
#include "warptrap/wtrap.h"

#include <stdio.h>
#include <stdlib.h>

#define TOT_WTS 1
static WTrap wt[TOT_WTS];

#define PREV_H 800
#define PREV_W 300
CvMat *invt;

void init_wts(void);
void main_mouseHandler(int event, int x, int y, int flags, void *param);
CvMat *build_transf_mat(WTrap *w, CvMat *mm);

int main(int argc, char *argv[]) {
	Uint32 nwidth, nheight;
	IplImage *oimg; // Original image;
	IplImage *mw_img; // Main window resized image

	invt = cvCreateMat(3, 3, CV_32FC1);

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

	init_wts();

	nwidth = oimg->width;
	nheight = oimg->height;

	recalc_img_size(&nwidth, &nheight, 800);
	mw_img = cvCreateImage(cvSize(nwidth , nheight), oimg->depth, oimg->nChannels); // Create a resized image
	cvResize(oimg, mw_img, CV_INTER_NN); // Resize

	// Create main window
	cvNamedWindow(MAIN_WIN, CV_WINDOW_AUTOSIZE); 

	// Show resized image
	update_wt_win(MAIN_WIN, mw_img, wt, cvScalar(0, 0, 255, 0));

	// Register mouse handler for main window
	cvSetMouseCallback(MAIN_WIN, main_mouseHandler, (void*)mw_img);

	// wait for a key
  	cvWaitKey(0);

	// Destroy main window
	cvDestroyWindow(MAIN_WIN);

	cvReleaseImage(&oimg); // Release greyscale image
	cvReleaseImage(&mw_img);  

	cvReleaseMat(&invt);

	return 0;
}

void main_mouseHandler(int event, int x, int y, int flags, void *param) {
	static Uint8 lb_down = 0;
	static Sint32 curnode = -1;

	static int oldx, oldy;
	int xdiff, ydiff;

	switch(event) {
		case CV_EVENT_LBUTTONDOWN:
			check_wtrap_point(x, y, &wt[0], &curnode);
			lb_down = 1;
			oldx = x;
			oldy = y;
			break;
		case CV_EVENT_LBUTTONUP:
			lb_down = 0;
			if (curnode >= 0) // And in this case we should update a preview window...
				invt = build_transf_mat(&wt[0], invt);
			
			curnode = -1;
			break;
		case CV_EVENT_MOUSEMOVE:
			if (curnode < 0 || !lb_down) break;

			xdiff = x - oldx;
			ydiff = y - oldy;

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

			oldx = x;
			oldy = y;

			update_wt_win(MAIN_WIN, (IplImage*)param, wt, cvScalar(0, 0, 255, 0));

			break;
		default:
			break;
	}

	return;
}

CvMat *build_transf_mat(WTrap *w, CvMat *mm) {
	// SEE cvWarpPerspectiveQMatrix
	// here: http://www.comp.leeds.ac.uk/vision/opencv/opencvref_cv.html

	CvPoint2D32f src[4];
	CvPoint2D32f dst[4];

	src[0].x = w->a.x;
	src[0].y = w->a.y;
	src[1].x = w->b.x;
	src[1].y = w->b.y;
	src[2].x = w->c.x;
	src[2].y = w->c.y;
	src[3].x = w->d.x;
	src[3].y = w->d.y;

	dst[0].x = 0;
	dst[0].y = 0;
	dst[1].x = PREV_W;
	dst[1].y = 0;
	dst[2].x = PREV_W;
	dst[2].y = PREV_H;
	dst[3].x = 0;
	dst[3].y = PREV_H;

	return cvWarpPerspectiveQMatrix(src, dst, mm);
}

void init_wts(void) {
	Uint16 i;

	Sint32 x = 40;
	Sint32 y = 20;

	for (i = 0; i < TOT_WTS; i++) {
		wt[i].a.x = x;
		wt[i].a.y = y;

		wt[i].b.x = x + 90;
		wt[i].b.y = y;

		wt[i].c.x = x + 90;
		wt[i].c.y = y + 180;

		wt[i].d.x = x;
		wt[i].d.y = y + 180;
	}
}
