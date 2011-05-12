#include "utils.h"

#include "spotclear/spotclear.h"

CvRect *getRoiFromPic(IplImage *in, Sint32 *tot_rois, Uint32 wmult, int agg);

IplImage *whiteThresh(IplImage *in, Uint8 thresh, Uint8 tolerance, Uint8 inv) {
	IplImage *ths = cvCreateImage(cvGetSize(in), 8, 1);
	
	Uint8 *in_dat = in->imageData;
	Uint8 *ths_dat = ths->imageData;
	Sint32 rgb[3], diff, med, i, j;

	Uint8 back, fore;

	if (inv) {
		back = 255;
		fore = 0;
	} else {
		back = 0;
		fore = 255;
	}

	for (i = 0; i < in->height; i++)
		for (j = 0; j < in->width; j++) {
			rgb[0] = in_dat[(i * in->widthStep) + (j * in->nChannels) + 0];
			rgb[1] = in_dat[(i * in->widthStep) + (j * in->nChannels) + 1];
			rgb[2] = in_dat[(i * in->widthStep) + (j * in->nChannels) + 2];	

			diff = abs(rgb[0] - rgb[1]) + abs(rgb[1] - rgb[2]) + abs(rgb[0] - rgb[2]);
			med = (rgb[0] + rgb[1] + rgb[2]) / 3;

			if (diff <= tolerance && med >= thresh)
				ths_dat[(i * ths->widthStep) + (j * ths->nChannels) + 0] = fore;
			else
				ths_dat[(i * ths->widthStep) + (j * ths->nChannels) + 0] = back;

		}

	return ths;
}

CvMat *build_transf_mat(WTrap *w, CvMat *mm, IplImage *or, IplImage *pw, Uint32 dwidth, Uint32 dheight) {
	// SEE cvWarpPerspectiveQMatrix
	// here: http://www.comp.leeds.ac.uk/vision/opencv/opencvref_cv.html

	float mx, my;

	mx = ((float)or->width / (float)pw->width);
	my = ((float)or->height / (float)pw->height);

	CvPoint2D32f src[4];
	CvPoint2D32f dst[4];

	src[0].x = w->a.x * mx;
	src[0].y = w->a.y * my;
	src[1].x = w->b.x * mx;
	src[1].y = w->b.y * my;
	src[2].x = w->c.x * mx;
	src[2].y = w->c.y * my;
	src[3].x = w->d.x * mx;
	src[3].y = w->d.y * my;

	dst[0].x = 0;
	dst[0].y = 0;
	dst[1].x = dwidth;
	dst[1].y = 0;
	dst[2].x = dwidth;
	dst[2].y = dheight;
	dst[3].x = 0;
	dst[3].y = dheight;

	return cvGetPerspectiveTransform(dst, src, mm);
}

IplImage *return_warped_img(IplImage *oim, CvMat *tm, WTrap *wt, Uint32 dwidth, Uint32 dheight, Sint8 chan) {
	assert(oim);
	assert(tm);

	IplImage *d1 = cvCreateImage(cvSize(dwidth, dheight), oim->depth, oim->nChannels);
	IplImage *dmono;

	cvWarpPerspective(oim, d1, tm, CV_INTER_CUBIC | CV_WARP_FILL_OUTLIERS | CV_WARP_INVERSE_MAP, cvScalarAll(0));
	
	if (chan >= 0) {
		dmono = gray_from_colour(d1,chan);
		cvReleaseImage(&d1);
	} else {
		dmono = d1;
	}

	return dmono;
}

Uint8 get_medium_intensity(IplImage *in, Uint8 chan) {
	Sint32 i, j;
	Uint32 intensity = 0;
	Uint8 *in_dat = in->imageData;
	Uint8 fint;

	for (i = 0; i < in->height; i++)
		for (j = 0; j < in->width; j++) {
				intensity += in_dat[(i * in->widthStep) + (j * in->nChannels) + chan];
		}

	fint = intensity / (in->height * in->width);

	return fint;
}

void recalc_img_size(Uint32 *width, Uint32 *height, Uint32 theight) {
	assert(theight);

	float ratio = (float)theight / (float)*height;

	*height = theight;
	*width = *width * ratio;

	*width = *width == 0 ? 1 : *width;
}

IplImage *gray_from_colour(IplImage *in, Uint8 chan) {
	assert(chan < 3);

	IplImage *grey = cvCreateImage(cvGetSize(in), 8, 1);
	
	Sint32 i, j;
	Uint8 *im_dat = in->imageData;
	Uint8 *m_dat = grey->imageData;

	int rgb[3];
	for (i = 0; i < in->height; i++)
		for (j = 0; j < in->width; j++) {
			rgb[0] = im_dat[(i * in->widthStep) + (j * in->nChannels) + 0];
			rgb[1] = im_dat[(i * in->widthStep) + (j * in->nChannels) + 1];
			rgb[2] = im_dat[(i * in->widthStep) + (j * in->nChannels) + 2];

			m_dat[(i * grey->widthStep) + (j * grey->nChannels) + 0] = rgb[chan];
		}

	return grey;
}

IplImage *anastize_image(IplImage *wimg, int msize, double mrem, Uint32 wmult, int agg) {
	assert(wimg);

	Sint32 tot_rois, i;
	IplImage *tmpi;
	CvRect *rois;

	IplImage *mimg = cvCreateImage(cvGetSize(wimg), 8, 1);	

	fprintf(stdout, " Applying local thresholding to image...\n");
	cvAdaptiveThreshold(wimg, mimg, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, msize, mrem);

#ifdef DEBUG
	cvSaveImage("./fstep0.jpg", mimg, 0);
#endif

if (agg > 3) {
	rois = getRoiFromPic(wimg, &tot_rois, wmult, agg - 3);

	tmpi = cvCreateImage(cvGetSize(mimg), mimg->depth, mimg->nChannels);
	cvRectangle(tmpi, cvPoint(0, 0), cvPoint(tmpi->width - 1, tmpi->height - 1), cvScalar(255, 255, 255, 0), CV_FILLED, 8, 0);

	for (i = 0; i < tot_rois; i++) {
		cvSetImageROI(mimg, rois[i]);
		cvSetImageROI(tmpi, rois[i]);
		cvCopy(mimg, tmpi, NULL);
	}
	cvResetImageROI(mimg);
	cvResetImageROI(tmpi);

	cvReleaseImage(&mimg);
	free(rois);
	mimg = tmpi;

#ifdef DEBUG
	cvSaveImage("./fstep1.jpg", mimg, 0);
#endif
}

if (agg > 0) {
	remove_spot_size(mimg, 1, 1 * wmult, Conn8); // Do a spot cleanup based on size
#ifdef DEBUG
	cvSaveImage("./fstep2.jpg", mimg, 0);
#endif
}

if (agg > 1) {
	remove_spot_neighbour_dist(mimg, 1, 2 * wmult, 3 * wmult, Conn8); // Do a cleanup based on distance
#ifdef DEBUG
	cvSaveImage("./fstep3.jpg", mimg, 0);
#endif
}

if (agg > 2) {
	remove_spot_neighbour_dist(mimg, 2 * wmult + 1, 8 * wmult, 8 * wmult, Conn8); 
#ifdef DEBUG
	cvSaveImage("./fstep4.jpg", mimg, 0);
#endif
}

	return mimg;
}

CvRect *getRoiFromPic(IplImage *in, Sint32 *tot_rois, Uint32 wmult, int agg) {
	assert(in);
	assert(tot_rois);
	assert(agg > 0);

	double thval = 230;
	int smoothval = 3;
	int erodval = 6;

	thval -= agg * 10;
	smoothval += agg;
	erodval -= agg;

	thval = thval < 0 ? 0 : thval;
	erodval = erodval <= 0 ? 1 : erodval;

	Uint32 nwidth, nheight;
	float xratio, yratio;
	nwidth = in->width;
	nheight = in->height;
	recalc_img_size(&nwidth, &nheight, 512);

	xratio = in->width / nwidth;
	yratio = in->height / nheight;

	// Use a very small image (256 pix height) for get ROIs
	IplImage *wpic = cvCreateImage(cvSize(nwidth , nheight), in->depth, in->nChannels);
	cvResize(in, wpic, CV_INTER_NN);
	Uint8 *wpic_dat = wpic->imageData;
	Sint32 max_rects = 1024;
	Sint32 trois = -1;
	CvRect *drois = (CvRect*)malloc(sizeof(CvRect) * max_rects);
	Sint32 xmin, xmax, i, j, whites;

	xmin = xmax = -1;

	cvAdaptiveThreshold(wpic, wpic, 255, /*CV_ADAPTIVE_THRESH_MEAN_C*/CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 16 * wmult + 1, 40);
	
	cvSmooth(wpic, wpic, CV_BLUR, smoothval, 0, 0, 0); // Smooth the input image, so only blobs remain
#ifdef DEBUG
	cvSaveImage("./getRoi-smooth.jpg", wpic, 0);
#endif
	
	cvErode(wpic, wpic, NULL, erodval);
#ifdef DEBUG
	cvSaveImage("./getRoi-erode.jpg", wpic, 0);
#endif

	cvThreshold(wpic, wpic, thval, 255, CV_THRESH_BINARY);
#ifdef DEBUG
	cvSaveImage("./getRoi-thresh.jpg", wpic, 0);
#endif

	// Go through the image
	for (i = 0; i < wpic->height; i += 2) {
		whites = 0;
		xmin = xmax = -1;
		for (j = 0; j < wpic->width; j++) {
			if(wpic_dat[(i * wpic->widthStep) + (j * wpic->nChannels) + 0] == 0) {
				if (xmin == -1) xmin = j;
				xmax = j;
				whites = 0;
			} else if (xmin != -1) {
				whites++;

				if (whites >= 60 && trois + 1 < max_rects) {
					drois[trois + 1].x = xmin;
					drois[trois + 1].y = i;
					drois[trois + 1].width = xmax-xmin;
					drois[trois + 1].height = MIN(2, wpic->height - i);

					drois[trois + 1].x *= xratio;
					drois[trois + 1].y *= yratio;
					drois[trois + 1].width *= xratio;
					drois[trois + 1].height *= yratio;

					trois++;
					
					whites = 0;
					xmin = xmax = -1;
				}
			}
		}

		if (xmin != -1 && (trois + 1) < max_rects) { // We found a rect!
			drois[trois + 1].x = xmin;
			drois[trois + 1].y = i;
			drois[trois + 1].width = xmax-xmin;
			drois[trois + 1].height = MIN(2, wpic->height - i);

			drois[trois + 1].x *= xratio;
			drois[trois + 1].y *= yratio;
			drois[trois + 1].width *= xratio;
			drois[trois + 1].height *= yratio;

			trois++;
		}
	}

	*tot_rois = trois;
	cvReleaseImage(&wpic);
	return drois;
}
