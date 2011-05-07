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
	cvSaveImage("./01testresi.jpg", smimg, 0);

	cvSmooth(smimg, smimg, CV_BLUR, 3, 0, 0, 0);
	cvSaveImage("./02testsmooth.jpg", smimg, 0);
	cvEqualizeHist(smimg, smimg);
	cvSaveImage("./03testhist.jpg", smimg, 0);
	cvThreshold(smimg, smimg, 180, 255, CV_THRESH_BINARY_INV);
	cvSaveImage("./04testthres.jpg", smimg, 0);
//	cvErode(smimg, smimg, NULL, 3);
	cvDilate(smimg, smimg, NULL, 1);
	cvSaveImage("./05testdil.jpg", smimg, 0);

	CvRect box;
	find_biggest_blob(smimg, &box);
	box.x *= xratio;
	box.y *= yratio;
	box.width *= xratio;
	box.height *= yratio;
	fprintf(stdout, "Biggest blobs is contained in a box starting at [%dx%d], %d pix wide and %d pix tall\n", box.x, box.y, box.width, box.height);

	cvRectangleR(oimg, box, cvScalar(255, 255, 255, 0), 1, 8, 0);
	cvSaveImage("./bookfound.jpg", oimg, 0);

	cvReleaseImage(&smimg);
	cvReleaseImage(&oimg);
	return 0;
}

