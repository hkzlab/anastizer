#include "common/defs.h"

int main(int argc, char *argv[]) {
	IplImage *oimg; // Original image;

	if (!(oimg = cvLoadImage(argv[1], CV_LOAD_IMAGE_UNCHANGED))) {
		fprintf(stderr, "Unable to load image %s\n", argv[1]);
		return 1;
	} else {
		fprintf(stdout, "Loaded image %s\n", argv[1]);
	}

	cvReleaseImage(&oimg); // Release greyscale image

	return 0;
}
