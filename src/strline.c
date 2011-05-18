#include "common/defs.h"

#define STR_BUF_SIZE 256

void hkzBaseMorph(const IplImage *src, IplImage *dst, IplConvKernel *se, Uint8 func, Uint32 iterations);

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

	int ckval[9] = {1, 1, 1, 1, 1 ,1 ,1 ,1 ,1};
	IplConvKernel *ck = cvCreateStructuringElementEx(3, 3, 0, 0, CV_SHAPE_CUSTOM, ckval);
	hkzBaseMorph(oimg, tst, ck, 1, 1);
	cvSaveImage("./test0.jpg", tst, 0);
	cvDilate(oimg, tst, ck, 1);
	cvSaveImage("./test1.jpg", tst, 0);
	cvReleaseStructuringElement(&ck);

	cvReleaseImage(&oimg);

	return 0;
}


void hkzBaseMorph(const IplImage *src, IplImage *dst, IplConvKernel *se, Uint8 func, Uint32 iterations) {
	assert(src);
	assert(dst);
	assert(se);
	assert(iterations > 0);
	assert(src->width == dst->width && src->height == dst->height);
	assert(src->depth == dst->depth && src->nChannels == dst->nChannels);
	assert(src->depth == 8);

	Uint32 ci;
	Sint32 i, j, n;
	Sint32 sei, sej, ax, ay;
	Uint8 good_el, cval, cont;
	Sint32 seval;

	Sint32 wpad = MAX(se->nCols - (se->nCols - se->anchorX), se->nCols - se->anchorX);
	Sint32 hpad = MAX(se->nRows - (se->nRows - se->anchorY), se->nRows - se->anchorY);
	
	IplImage *tmpia = cvCreateImage(cvSize(src->width + wpad * 2, src->height + hpad * 2), src->depth, src->nChannels);
	cvSet(tmpia, cvColorToScalar(0, cvGetElemType(tmpia)), 0);
	IplImage *tmpib = cvCloneImage(tmpia);
	IplImage *imswap;

	cvSetImageROI(tmpia, cvRect(wpad, hpad, src->width, src->height));
	cvCopy(src, tmpia, 0);
	cvResetImageROI(tmpia);

	for (ci = 0; ci < iterations; ci++) { // Iterate!
		for (i = wpad; i < tmpia->width - wpad; i++)
			for (j = hpad; j < tmpia->height - hpad; j++)
				for (n = 0; n < tmpia->nChannels; n++) {
					if (func == 1) // Dilate
						good_el = 0;
					else // Erode
						good_el = 1;
					
					ax = i - se->anchorX;
					ay = j - se->anchorY;

					cont = 1;
					for (sei = 0; sei < se->nCols && cont; sei++)
						for (sej = 0; sej < se->nRows && cont; sej++) {
							seval = se->values[sej * se->nCols + sei];
							if (seval == 0) continue;
							else if (seval < 0) { // This element in the image must be 0
								cval = tmpia->imageData[((sej + ay) * tmpia->widthStep) + ((sei + ax) * tmpia->nChannels) + n];
							
								if (cval > 0 && func != 1) { // Eroding... pixel isn't good
									good_el = 0;
									cont = 0;
								} else if (cval == 0 && func == 1) { // Dilating... OK
									good_el = 1;
									cont = 0;
								}

							} else { // > 0, this MUST be 255
								cval = tmpia->imageData[((sej + ay) * tmpia->widthStep) + ((sei + ax) * tmpia->nChannels) + n];

								if (cval < 255 && func != 1) { // Eroding... pixel isn't good
									good_el = 0;
									cont = 0;
								} else if (cval == 255 && func == 1) { // Dilating... OK
									good_el = 1;
									cont = 0;
								}
							}

						}
					// Iterate through the SE and check if pix is good
			
					if (good_el)
						tmpib->imageData[(j * tmpib->widthStep) + (i * tmpib->nChannels) + n] = 255;
				}

		imswap = tmpib;
		tmpib = tmpia;
		tmpia = imswap;
		cvSet(tmpib, cvColorToScalar(0, cvGetElemType(tmpia)), 0);
	}

	// Copy image to destination
	cvSetImageROI(tmpia, cvRect(wpad, hpad, src->width, src->height));
	cvCopy(tmpia, dst, 0);
	cvResetImageROI(tmpia);

	cvReleaseImage(&tmpia);
	cvReleaseImage(&tmpib);
}
