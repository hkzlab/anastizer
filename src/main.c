#include "common/defs.h"
#include "common/win_names.h"
#include "utils/utils.h"
#include "warptrap/wtrap.h"

#include <stdio.h>
#include <stdlib.h>

void main_mouseHandler(int event, int x, int y, int flags, void *param);

int main(int argc, char *argv[]) {
	Uint32 nwidth, nheight;
	IplImage *oimg; // Original image;
	IplImage *mw_img; // Main window resized image

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

	nwidth = oimg->width;
	nheight = oimg->height;

	recalc_img_size(&nwidth, &nheight, 800);
	mw_img = cvCreateImage(cvSize(nwidth , nheight), oimg->depth, oimg->nChannels); // Create a resized image
	cvResize(oimg, mw_img, CV_INTER_NN); // Resize

	// Create main window
	cvNamedWindow(MAIN_WIN, CV_WINDOW_AUTOSIZE); 

	// Show resized image
	cvShowImage(MAIN_WIN, mw_img);

	// Register mouse handler for main window
	cvSetMouseCallback(MAIN_WIN, main_mouseHandler, NULL);

	// wait for a key
  	cvWaitKey(0);

	// Destroy main window
	cvDestroyWindow(MAIN_WIN);

	cvReleaseImage(&oimg); // Release greyscale image
	cvReleaseImage(&mw_img);  

	return 0;
}

void main_mouseHandler(int event, int x, int y, int flags, void *param) {
	return;
}
