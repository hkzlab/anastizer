#include "common/defs.h"
#include "morphology/morph.h"

#define STR_BUF_SIZE 256

int main(int argc, char *argv[]) {
	IplImage *oimg;

	char src_file[STR_BUF_SIZE];
	src_file[0] = '\0';

	// Prepare file path, removing last 4 chars (extension)
	strncat(src_file, argv[1], strlen(argv[1]) - 4);

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

	cvThreshold(oimg, oimg, 128, 255, CV_THRESH_BINARY_INV); // Make sure the image is binary!

	IplImage *tst = cvCloneImage(oimg);

	int ckval[9] = {-1, -1, 0, -1, 1, 0, 0, 0, 0};
	IplConvKernel *ck = cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_CUSTOM, ckval);
	hkzBaseMorph(oimg, tst, ck, HKZ_ERODE, 1);
	cvSaveImage("./test0.jpg", tst, 0);
	cvReleaseStructuringElement(&ck);

	cvReleaseImage(&oimg);

	return 0;
}


