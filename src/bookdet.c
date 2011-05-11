#include "common/defs.h"
#include "common/bookd_defs.h"
#include "utils/utils.h"
#include "spotclear/spotclear.h"
#include "imc/imc.h"

double get_optimum_angle(IplImage *img);

int main(int argc, char *argv[]) {
	Uint32 nwidth, nheight, spotsize;
	IplImage *oimg, *smimg, *rot_img;
	CvMat *rot_mat;
	Sint32 i, j, k, fblack, lblack, xmin, xmax;
	CvRect box;
	double xratio, yratio, optangle;
	float da, db, dc, dd, dp;

	CvPoint2D32f center;
	CvPoint a, b, c, d, ca, cb;

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

	// Produce a very small working image
	nwidth = oimg->width;
	nheight = oimg->height;
	recalc_img_size(&nwidth, &nheight, 256);
	smimg = cvCreateImage(cvSize(nwidth , nheight), oimg->depth, oimg->nChannels);
	cvResize(oimg, smimg, CV_INTER_NN); // Create a very small preview image
	xratio = oimg->width / (float)smimg->width;
	yratio = oimg->height / (float)smimg->height;
	cvSaveImage("./01testresi.jpg", smimg, 0);

	// Smooth, threshold and erode the small image
	cvSmooth(smimg, smimg, CV_BLUR, 4, 0, 0, 0);
	cvSaveImage("./02testsmooth.jpg", smimg, 0);
	IplImage *timg = whiteThresh(smimg, 120, 25, 1);
	cvSaveImage("./04testthres.jpg", timg, 0);
	cvErode(timg, timg, NULL, 1);
	cvSaveImage("./05testdil.jpg", timg, 0);

	// Find where the book probably is and clean the rest
	spotsize = find_biggest_blob(timg, &box, Conn4);
	remove_spot_size(timg, 1, spotsize - 1, Conn4);
	cvSaveImage("./06testrem.jpg", timg, 0);

	// Fill the white blobs inside the book silouette
	for (j = 0; j < timg->width; j++) {
		fblack = -1;
		lblack = -1;

		for (i = 0; i < timg->height; i++) {
			if (!timg->imageData[(i * timg->widthStep) + (j * timg->nChannels) + 0]) {
				if (fblack < 0) {
					fblack = lblack = i;
				} else lblack = i;
			}
		}

		if (fblack > 0)
			cvLine(timg, cvPoint(j, fblack), cvPoint(j, lblack), cvScalarAll(0), 1, 8, 0);
	}

	// Try to find the optimum rotation angle for the book
	optangle = get_optimum_angle(timg);
	fprintf(stdout, "opt. rot. angle %f\n", optangle);

	// Rotate the small image
	rot_img = cvCloneImage(timg);
	center.x = box.x + box.width / 2;
	center.y = box.y + box.height / 2;
	rot_mat = cvCreateMat(2, 3, CV_32FC1);
	rot_mat = cv2DRotationMatrix(center, optangle, 1.0, rot_mat);
	cvWarpAffine(timg, rot_img, rot_mat, CV_INTER_NN | CV_WARP_FILL_OUTLIERS, cvScalarAll(255));
	cvReleaseMat(&rot_mat);

	cvSaveImage("./savedbw.jpg", rot_img, 0);


	find_biggest_blob(rot_img, &box, Conn4);

	// Resize the top part b
	xmin = box.x;
	xmax = box.width - 1;
	for (j = box.x; j < box.x + box.width - 1; j++)
		for (i = box.y; i < box.y + box.height / 2; i++) {
			if (!rot_img->imageData[(i * rot_img->widthStep) + (j * rot_img->nChannels) + 0]) {
				xmax = MAX(xmax, j);
				xmin = MIN(xmin, j);
			}
		}

	box.x = xmin;
	box.width = xmax - xmin;
	box.height = box.height / 2;

	// Make sure the width is odd
	if (!(box.width % 2)) box.width--;

	float val_dec = 1.0 / (box.width / 2);
	float prob_val = 1.0;
	Uint32 step, min, lval, rval;
	float minval = 1.0;

	// Try to find the CENTER of the book
	step = lval = rval = xmin = 0;
	min = 0xFFFFFFFF;
	for (j = box.x + box.width / 2 + 1; j < box.x + box.width - 1 && prob_val > 0.5; j++) {
		for (i = box.y; i < box.y + box.height - 1; i++) {
			if (!rot_img->imageData[(i * rot_img->widthStep) + (((box.x + box.width / 2 + 1) - step) * rot_img->nChannels) + 0])
				lval++;
			if (!rot_img->imageData[(i * rot_img->widthStep) + (((box.x + box.width / 2 + 1) + step) * rot_img->nChannels) + 0])
				rval++;
		}

		if (min > lval && lval > 10) {
			min = lval;
			minval = prob_val;
			xmin = (box.x + box.width / 2 + 1) - step;
		}

		if (min > rval && rval > 10) {
			min = rval;
			minval = prob_val;
			xmin = (box.x + box.width / 2 + 1) + step;
		}

		step++;
		prob_val -= val_dec;
		lval = rval = 0;
	}

	// Calculate the coords of the top and bottom center points
	ca.x = cb.x = xmin;

	fblack = -1;
	lblack = -1;
	for (i = 0; i < rot_img->height; i++) {
		if (!rot_img->imageData[(i * rot_img->widthStep) + (xmin * rot_img->nChannels) + 0]) {
			if (fblack < 0) {
				fblack = lblack = i;
			} else lblack = i;
		}
	}

	ca.y = fblack;
	cb.y = lblack;

	// Try to find BORDER points
	find_biggest_blob(timg, &box, Conn4);

	a.x = b.x = c.x = d.x = box.x + box.width / 2;
	a.y = b.y = c.y = d.y = box.y + box.height / 2;

	da = sqrtf(powf(a.x - box.x, 2) + powf(a.y - box.y, 2));
	db = sqrtf(powf(b.x - (box.x + box.width - 1), 2) + powf(b.y - box.y, 2));
	dc = sqrtf(powf(c.x - box.x, 2) + powf(c.y - (box.y + box.height - 1), 2));
	dd = sqrtf(powf(d.x - (box.x + box.width - 1), 2) + powf(d.y - (box.y + box.height - 1), 2));

	for (j = box.x; j < box.x + box.width - 1; j++)
		for (i = box.y; i < box.y + box.height - 1; i++) {
			if (!rot_img->imageData[(i * rot_img->widthStep) + (j * rot_img->nChannels) + 0]) {
				if (j == 0 || j == rot_img->width - 1 || i == 0 || i == rot_img->height || rot_img->imageData[((i - 1) * rot_img->widthStep) + (j * rot_img->nChannels) + 0] || \
				        rot_img->imageData[((i + 1) * rot_img->widthStep) + (j * rot_img->nChannels) + 0] || rot_img->imageData[(i * rot_img->widthStep) + ((j - 1) * rot_img->nChannels) + 0] || \
				        rot_img->imageData[(i * rot_img->widthStep) + ((j + 1) * rot_img->nChannels) + 0]) {

					dp = sqrtf(powf(j - box.x, 2) + powf(i - box.y, 2));
					if (dp < da) {
						da = dp;
						a.x = j;
						a.y = i;
					}

					dp = sqrtf(powf(j - (box.x + box.width - 1), 2) + powf(i - box.y, 2));
					if (dp < db) {
						db = dp;
						b.x = j;
						b.y = i;
					}

					dp = sqrtf(powf(j - box.x, 2) + powf(i - (box.y + box.height - 1), 2));
					if (dp < dc) {
						dc = dp;
						c.x = j;
						c.y = i;
					}

					dp = sqrtf(powf(j - (box.x + box.width - 1), 2) + powf(i - (box.y + box.height - 1), 2));
					if (dp < dd) {
						dd = dp;
						d.x = j;
						d.y = i;
					}
				}
			}
		}

	// We should now scale the points and rotate them back
	a.x *= xratio; b.x *= xratio; c.x *= xratio; d.x *= xratio; ca.x *= xratio; cb.x *= xratio;
	a.y *= yratio; b.y *= yratio; c.y *= yratio; d.y *= yratio; ca.y *= yratio; cb.y *= yratio;

	center.x *= xratio;
	center.y *= yratio;

	rot_mat = cvCreateMat(2, 3, CV_32FC1);
	CvMat *pntm = cvCreateMat(3, 1, CV_32FC1);
	CvMat *dpntm = cvCreateMat(2, 1, CV_32FC1);
	rot_mat = cv2DRotationMatrix(center, -1.0 * optangle, 1.0, rot_mat);

	//
	cvmSet(pntm, 0, 0, a.x);
	cvmSet(pntm, 1, 0, a.y);
	cvmSet(pntm, 2, 0, 1.0);

	cvMatMul(rot_mat, pntm, dpntm);

	a.x = round(cvmGet(dpntm, 0, 0));
	a.y = round(cvmGet(dpntm, 1, 0));

	//
	cvmSet(pntm, 0, 0, b.x);
	cvmSet(pntm, 1, 0, b.y);
	cvmSet(pntm, 2, 0, 1.0);

	cvMatMul(rot_mat, pntm, dpntm);

	b.x = round(cvmGet(dpntm, 0, 0));
	b.y = round(cvmGet(dpntm, 1, 0));

	//
	cvmSet(pntm, 0, 0, c.x);
	cvmSet(pntm, 1, 0, c.y);
	cvmSet(pntm, 2, 0, 1.0);

	cvMatMul(rot_mat, pntm, dpntm);

	c.x = round(cvmGet(dpntm, 0, 0));
	c.y = round(cvmGet(dpntm, 1, 0));

	//
	cvmSet(pntm, 0, 0, d.x);
	cvmSet(pntm, 1, 0, d.y);
	cvmSet(pntm, 2, 0, 1.0);

	cvMatMul(rot_mat, pntm, dpntm);

	d.x = round(cvmGet(dpntm, 0, 0));
	d.y = round(cvmGet(dpntm, 1, 0));

	//
	cvmSet(pntm, 0, 0, ca.x);
	cvmSet(pntm, 1, 0, ca.y);
	cvmSet(pntm, 2, 0, 1.0);

	cvMatMul(rot_mat, pntm, dpntm);

	ca.x = round(cvmGet(dpntm, 0, 0));
	ca.y = round(cvmGet(dpntm, 1, 0));

	//
	cvmSet(pntm, 0, 0, cb.x);
	cvmSet(pntm, 1, 0, cb.y);
	cvmSet(pntm, 2, 0, 1.0);

	cvMatMul(rot_mat, pntm, dpntm);

	cb.x = round(cvmGet(dpntm, 0, 0));
	cb.y = round(cvmGet(dpntm, 1, 0));

	cvCircle(oimg, a, 5, cvScalar(0, 0, 255, 0), 3, 8, 0);
	cvCircle(oimg, b, 5, cvScalar(0, 0, 255, 0), 3, 8, 0);
	cvCircle(oimg, c, 5, cvScalar(0, 0, 255, 0), 3, 8, 0);
	cvCircle(oimg, d, 5, cvScalar(0, 0, 255, 0), 3, 8, 0);
	cvCircle(oimg, ca, 5, cvScalar(0, 0, 255, 0), 3, 8, 0);
	cvCircle(oimg, cb, 5, cvScalar(0, 0, 255, 0), 3, 8, 0);

	cvSaveImage("./outpoints.jpg", oimg, 0);

	cvReleaseMat(&pntm);
	cvReleaseMat(&dpntm);
	cvReleaseMat(&rot_mat);

	cvReleaseImage(&rot_img);
	cvReleaseImage(&smimg);
	cvReleaseImage(&timg);
	cvReleaseImage(&oimg);

	// Save the points
	imc_data *dt = allocImcData(2);
	dt->wt[0].a.x = a.x;
	dt->wt[0].a.y = a.y;

	dt->wt[0].b.x = ca.x;
	dt->wt[0].b.y = ca.y;

	dt->wt[0].c.x = cb.x;
	dt->wt[0].c.y = cb.y;

	dt->wt[0].d.x = d.x;
	dt->wt[0].d.y = d.y;

	dt->wt[1].a.x = ca.x;
	dt->wt[1].a.y = ca.y;

	dt->wt[1].b.x = b.x;
	dt->wt[1].b.y = b.y;

	dt->wt[1].c.x = c.x;
	dt->wt[1].c.y = c.y;

	dt->wt[1].d.x = cb.x;
	dt->wt[1].d.y = cb.y;

	saveImcData("./test.imc", dt);
	freeImcData(&dt);

	return 0;
}

double get_optimum_angle(IplImage *img) {
	double cur_angle;
	Sint32 i, j;

	CvRect box;
	CvPoint2D32f cp;
	IplImage *cimg;
	CvMat *rot_mat;

	rot_mat = cvCreateMat(2, 3, CV_32FC1);
	find_biggest_blob(img, &box, Conn4);

	// Calculate blob approximate center
	cp.x = box.x + box.width / 2;
	cp.y = box.y + box.height / 2;

	Uint32 bsize;
	double obmed, bmed;
	bmed = obmed = 0.0;
	for (cur_angle = 0.0; cur_angle > -180.0; cur_angle -= 0.25) {
		cimg = cvCreateImage(cvGetSize(img), 8, 1);
		cv2DRotationMatrix(cp, cur_angle, 1.0, rot_mat);
		cvWarpAffine(img, cimg, rot_mat, CV_INTER_NN | CV_WARP_FILL_OUTLIERS, cvScalarAll(255));

		bsize = find_biggest_blob(cimg, &box, Conn4);
		bmed = bsize / (float)(box.width * box.height);

		//fprintf(stdout, "bmed %f, obmed %f\n", bmed, obmed);

		cvReleaseImage(&cimg);

		// 0.005 tolerance
		if (bmed < (obmed - 0.005) && box.width > box.height) break;
		else obmed = bmed;
	}

	cvReleaseMat(&rot_mat);

	return cur_angle - 0.25;
}
