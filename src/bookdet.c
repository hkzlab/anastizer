#include "common/defs.h"
#include "common/bookd_defs.h"
#include "utils/utils.h"

int main(int argc, char *argv[]) {
	Uint32 nwidth, nheight;
	IplImage *oimg, *tmp_img, *smimg;

	// Check parameters and load image file
	if (argc < 2) {
		fprintf(stdout, "%s imagefile\n", argv[0]);
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
	recalc_img_size(&nwidth, &nheight, ORIG_H);
	tmp_img = cvCreateImage(cvSize(nwidth , nheight), oimg->depth, oimg->nChannels);
	cvResize(oimg, tmp_img, CV_INTER_CUBIC);
	cvReleaseImage(&oimg);
	oimg = tmp_img; // Replace original image with the resized version

	nwidth = oimg->width;
	nheight = oimg->height;
	recalc_img_size(&nwidth, &nheight, 256);
	smimg = cvCreateImage(cvSize(nwidth , nheight), oimg->depth, oimg->nChannels);
	cvResize(oimg, smimg, CV_INTER_NN); // Create a very small preview image

	cvSmooth(smimg, smimg, CV_BLUR, 25, 0, 0, 0);
	cvThreshold(smimg, smimg, 190, 255, CV_THRESH_BINARY_INV);
	cvSaveImage("./test.jpg", smimg, 0);

	cvReleaseImage(&smimg);
	cvReleaseImage(&oimg);
	return 0;
}

