#include "utils.h"

#include "common/globs.h"
#include "spotclear/spotclear.h"

CvRect *getRoiFromPic(IplImage *in, Sint32 *tot_rois);

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

	return cvWarpPerspectiveQMatrix(dst, src, mm);
}

IplImage *return_warped_img(IplImage *oim, CvMat *tm, WTrap *wt, Uint32 dwidth, Uint32 dheight, Sint8 chan) {
	assert(oim);
	assert(tm);

	IplImage *d1 = cvCreateImage(cvSize(dwidth, dheight), oim->depth, oim->nChannels);
	IplImage *dmono;

	cvWarpPerspective(oim, d1, tm, CV_INTER_CUBIC + CV_WARP_FILL_OUTLIERS + CV_WARP_INVERSE_MAP, cvScalarAll(0));
	
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

IplImage *anastize_image(IplImage *wimg) {
	assert(wimg);

	Sint32 tot_rois, i;
	IplImage *tmpi;

	IplImage *mimg = cvCreateImage(cvGetSize(wimg), 8, 1);	

	fprintf(stdout, " Applying local thresholding to image...\n");
	cvAdaptiveThreshold(wimg, mimg, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, tmask_size, tmask_avr);

	CvRect *rois = getRoiFromPic(wimg, &tot_rois);

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
/*
	remove_spot_size(mimg, 1, 3 * WARP_MULT, Conn8); // Do a spot cleanup based on size
	remove_spot_intensity(mimg, wimg, 1, 8 * WARP_MULT, 15, 0, Conn8); // Do a cleanup based on intensity
	remove_spot_intensity(mimg, wimg, 8 * WARP_MULT + 1, 400 * WARP_MULT, 100, 0, Conn8);
	remove_spot_intensity(mimg, wimg, 400 * WARP_MULT + 1, 600 * WARP_MULT, 80, 0, Conn8);
	remove_spot_neighbour_dist(mimg, 1, 4 * WARP_MULT, 2 * WARP_MULT, Conn8); // Do a cleanup based on distance
	remove_spot_neighbour_dist(mimg, 4 * WARP_MULT + 1, 8 * WARP_MULT, 8 * WARP_MULT, Conn8); 
	remove_spot_neighbour_dist(mimg, 8 * WARP_MULT + 1, 150 * WARP_MULT, 30 * WARP_MULT, Conn8);
	remove_spot_thin(mimg, 1, 5 * WARP_MULT, 0.6, Conn8); // Do a cleanup based on thinness of the element

	
*/
	return mimg;
}

CvRect *getRoiFromPic(IplImage *in, Sint32 *tot_rois) {
	assert(in);
	assert(tot_rois);

	Uint32 nwidth, nheight, xratio, yratio;
	nwidth = in->width;
	nheight = in->height;
	recalc_img_size(&nwidth, &nheight, 300);

	xratio = in->width / nwidth;
	yratio = in->height / nheight;

	IplImage *wpic = cvCreateImage(cvSize(nwidth , nheight), in->depth, in->nChannels);
	cvResize(in, wpic, CV_INTER_CUBIC);
	//IplImage *wpic = cvCloneImage(in);
	Uint8 *wpic_dat = wpic->imageData;
	Sint32 max_rects = 1024;
	Sint32 trois = -1;
	CvRect *drois = (CvRect*)malloc(sizeof(CvRect) * max_rects);
	Sint32 xmin, xmax, i, j, whites;

	xmin = xmax = -1;

	cvAdaptiveThreshold(wpic, wpic, 255, /*CV_ADAPTIVE_THRESH_MEAN_C*/CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 15 * WARP_MULT + 1, 40);
	cvSmooth(wpic, wpic, CV_BLUR, 5, 0, 0, 0); // Smooth the input image, so only blobs remain
	cvThreshold(wpic, wpic, 200, 255, CV_THRESH_BINARY);

	cvSaveImage("./testroiblur.jpg", wpic, 0);

	cvErode(wpic, wpic, NULL, 10); // And erode it so we get BIG black squares in place of text

	cvSaveImage("./testroiero.jpg", wpic, 0);

	// Go through the image
	for (i = 0; i < wpic->height; i += 4) {
		whites = 0;
		xmin = xmax = -1;
		for (j = 0; j < wpic->width; j++) {
			if(wpic_dat[(i * wpic->widthStep) + (j * wpic->nChannels) + 0] == 0) {
				if (xmin == -1) xmin = j;
				xmax = j;
				whites = 0;
			} else if (xmin != -1) {
				whites++;

				if (whites >= 70 * WARP_MULT) {
					drois[trois + 1].x = xmin;
					drois[trois + 1].y = i;
					drois[trois + 1].width = xmax-xmin;
					drois[trois + 1].height = MIN(4, wpic->height - i);

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
			drois[trois + 1].height = MIN(4, wpic->height - i);

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
