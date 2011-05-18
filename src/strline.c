#include "common/defs.h"
#include "morphology/morph.h"
#include "spotclear/spotclear.h"

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

	// FIXME: cleanup this mess and write something decent here
	// This is just test code for now
	int ckval[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
	IplConvKernel *ck = cvCreateStructuringElementEx(6, 1, 3, 0, CV_SHAPE_CUSTOM, ckval);
	hkzBaseMorph(oimg, oimg, ck, HKZ_DILATE, 8);
	cvReleaseStructuringElement(&ck);

	ck = cvCreateStructuringElementEx(1, 5, 0, 2, CV_SHAPE_CUSTOM, ckval);
	hkzBaseMorph(oimg, oimg, ck, HKZ_ERODE, 2);
	cvSaveImage("./test0.jpg", oimg, 0);
	cvReleaseStructuringElement(&ck);

	ck = cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_CUSTOM, ckval);
	hkzBaseMorph(oimg, oimg, ck, HKZ_DILATE, 1);
	hkzBaseMorph(oimg, oimg, ck, HKZ_ERODE, 1);
	cvSaveImage("./test1.jpg", oimg, 0);
	cvReleaseStructuringElement(&ck);

	hkzMorphFullThin(oimg, oimg);
	cvSaveImage("./test2.jpg", oimg, 0);

	IplImage *rub = cvCloneImage(oimg);
	IplImage *rub2 = cvCloneImage(oimg);
	int bkval[9] = {-1, 1, 1, -1, 1, -1, 1, 1, -1};
	ck = cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_CUSTOM, bkval);
	hkzBaseMorph(oimg, rub, ck, HKZ_ERODE, 1);
	hkzBinPicSub(oimg, rub);
	cvSaveImage("./test3.jpg", rub, 0);

	int dkval[9] = {1, 1, -1, -1, 1, -1, -1, 1, 1};
	ck = cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_CUSTOM, dkval);
	hkzBaseMorph(rub, rub2, ck, HKZ_ERODE, 1);
	hkzBinPicSub(rub, rub2);
	cvSaveImage("./test4.jpg", rub2, 0);

	cvThreshold(rub, rub, 128, 255, CV_THRESH_BINARY_INV);
	remove_spot_size(rub, 1, 20, Conn4);
	cvThreshold(rub, rub, 128, 255, CV_THRESH_BINARY_INV); 

	cvSaveImage("./test5.jpg", rub, 0);

	cvReleaseImage(&rub);
	cvReleaseImage(&rub2);

	// Now we need to find spurs, purge them and maybe avoid different lines joining together

	cvReleaseImage(&oimg);

	return 0;
}


