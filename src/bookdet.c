#include "common/defs.h"
#include "common/bookd_defs.h"
#include "utils/utils.h"

int main(int argc, char *argv[]) {
	Uint32 nwidth, nheight;
	IplImage *oimg, *tmp_img;

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

	recalc_img_size(&nwidth, &nheight, ORIG_H);
	tmp_img = cvCreateImage(cvSize(nwidth , nheight), oimg->depth, oimg->nChannels);
	cvResize(oimg, tmp_img, CV_INTER_CUBIC);

	cvReleaseImage(&tmp_img);
	cvReleaseImage(&oimg);

	return 0;
}

