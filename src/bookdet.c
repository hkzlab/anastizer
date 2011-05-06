#include "common/defs.h"
#include "common/bookd_defs.h"
#include "utils/utils.h"
#include "spotclear/spotclear.h"

int main(int argc, char *argv[]) {
	Uint32 nwidth, nheight;
	IplImage *oimg, *tmp_img, *smimg;

	// Check parameters and load image file
	if (argc < 2) {
		fprintf(stdout, "%s imagefile\n", argv[0]);
		return 1;
	}

	if (!(oimg = cvLoadImage(argv[1], CV_LOAD_IMAGE_GRAYSCALE))) {
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

	float xratio, yratio;
	nwidth = oimg->width;
	nheight = oimg->height;
	recalc_img_size(&nwidth, &nheight, 256);
	smimg = cvCreateImage(cvSize(nwidth , nheight), oimg->depth, oimg->nChannels);
	cvResize(oimg, smimg, CV_INTER_NN); // Create a very small preview image
	xratio = oimg->width / smimg->width;
	yratio = oimg->height / smimg->height;

	cvSmooth(smimg, smimg, CV_BLUR, 8, 0, 0, 0);
	cvThreshold(smimg, smimg, 190, 255, CV_THRESH_BINARY_INV);
	cvDilate(smimg, smimg, NULL, 4);

	Sint32 bx, by, bwidth, bheight;
	find_biggest_blob(smimg, &bx, &by, &bwidth, &bheight);
	bx *= xratio;
	by *= yratio;
	bwidth *= xratio;
	bheight *= yratio;
	fprintf(stdout, "Biggest blobs is contained in a box starting at [%dx%d], %d pix wide and %d pix tall\n", bx, by, bwidth, bheight);

	cvSaveImage("./test.jpg", smimg, 0);

	cvReleaseImage(&smimg);
	cvReleaseImage(&oimg);
	return 0;
}

