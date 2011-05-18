#include "morph.h"

void hkzBaseMorph(const IplImage *src, IplImage *dst, IplConvKernel *se, enum Morph func, Uint32 iterations) {
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
					if (func == HKZ_DILATE) // Dilate
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
							
								if (cval > 0 && func == HKZ_ERODE) { // Eroding... pixel isn't good
									good_el = 0;
									cont = 0;
								} else if (cval == 0 && func == HKZ_DILATE) { // Dilating... OK
									good_el = 1;
									cont = 0;
								}

							} else { // > 0, this MUST be 255
								cval = tmpia->imageData[((sej + ay) * tmpia->widthStep) + ((sei + ax) * tmpia->nChannels) + n];

								if (cval < 255 && func == HKZ_ERODE) { // Eroding... pixel isn't good
									good_el = 0;
									cont = 0;
								} else if (cval == 255 && func == HKZ_DILATE) { // Dilating... OK
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

Sint8 hkzMorphComparePics(const IplImage *im1, const IplImage *im2) {
	assert(im1 && im2);
	assert(im1->depth == 8 && im2->depth == 8);

	if (im1->width != im2->width || im1->height != im2->height) return -1;

	Sint32 i, j, n;
	for (i = 0; i < im1->width; i++)
		for (j = 0; j < im1->height; j++)
			for (n = 0; n < im1->nChannels; n++)
				if (im1->imageData[(j * im1->widthStep) + (i * im1->nChannels) + n] != im2->imageData[(j * im2->widthStep) + (i * im2->nChannels) + n]) return -1;

	return 0;
}

void hkzBinPicSub(const IplImage *im1, IplImage *im2) {
	assert(im1 && im2);
	assert(im1->depth == 8 && im2->depth == 8);
	assert(im1->width == im2->width && im2->height == im1->height);

	Sint32 i, j;
	Uint8 *im1_data, *im2_data;

	im1_data = im1->imageData;
	im2_data = im2->imageData;

	for (i = 0; i < im1->width; i++)
		for (j = 0; j < im1->height; j++)
			im2_data[(j * im2->widthStep) + (i * im2->nChannels)] = \
				im2_data[(j * im2->widthStep) + (i * im2->nChannels)] && im1_data[(j * im1->widthStep) + (i * im1->nChannels)] ? 0 : im1->imageData[(j * im1->widthStep) + (i * im1->nChannels)];
}

void hkzMorphThin(const IplImage *src, IplImage *dst, IplConvKernel *se) {
	assert(src);
	assert(dst);
	assert(se);
	assert(src->depth == 8 && src->nChannels == 1);

	IplImage *tmpi = cvCloneImage(src);

	hkzBaseMorph(src, dst, se, HKZ_ERODE, 1);
	hkzBinPicSub(tmpi, dst);

	cvReleaseImage(&tmpi);
}
