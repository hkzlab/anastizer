#include "common/defs.h"
#include "common/bookd_defs.h"
#include "utils/utils.h"
#include "spotclear/spotclear.h"

double get_optimum_angle(IplImage *img);

int main(int argc, char *argv[]) {
	Uint32 nwidth, nheight;
	IplImage *oimg, *tmp_img, *smimg;

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

	cvSmooth(smimg, smimg, CV_BLUR, 4, 0, 0, 0);
	cvSaveImage("./02testsmooth.jpg", smimg, 0);
//	cvEqualizeHist(smimg, smimg);
//	cvSaveImage("./03testhist.jpg", smimg, 0);
//	cvThreshold(smimg, smimg, 160, 255, CV_THRESH_BINARY_INV);
	IplImage *timg = whiteThresh(smimg, 120, 25, 1);	
	cvSaveImage("./04testthres.jpg", timg, 0);
	cvErode(timg, timg, NULL, 1);
//	cvDilate(timg, timg, NULL, 5);
	cvSaveImage("./05testdil.jpg", timg, 0);

	CvRect box;
	Uint32 spotsize = find_biggest_blob(timg, &box, Conn4);
	remove_spot_size(timg, 1, spotsize - 1, Conn4);
	cvSaveImage("./06testrem.jpg", timg, 0);
	box.x *= xratio;
	box.y *= yratio;
	box.width *= xratio;
	box.height *= yratio;
	fprintf(stdout, "Biggest blobs is contained in a box starting at [%dx%d], %d pix wide and %d pix tall\n", box.x, box.y, box.width, box.height);

	cvRectangleR(oimg, box, cvScalar(0, 0, 255, 0), 2, 8, 0);
	cvSaveImage("./bookfound.jpg", oimg, 0);

	double optangle = get_optimum_angle(timg);
	fprintf(stdout, "optangle %f\n", optangle);

	IplImage *turnout = cvCloneImage(oimg);
	CvPoint2D32f center;
	center.x = box.x + box.width/2;
	center.y = box.y + box.height/2;
	CvMat* rot_mat = cvCreateMat(2, 3, CV_32FC1);
	rot_mat = cv2DRotationMatrix(center, optangle, 1.0, rot_mat);
	cvWarpAffine(oimg, turnout, rot_mat, CV_INTER_NN | CV_WARP_FILL_OUTLIERS, cvScalarAll(255));
	cvSaveImage("./saved.jpg", turnout, 0);
	cvReleaseMat(&rot_mat);
	cvReleaseImage(&turnout);

	cvReleaseImage(&smimg);
	cvReleaseImage(&timg);
	cvReleaseImage(&oimg);
	return 0;
}

double get_optimum_angle(IplImage *img) {
	double cur_angle;
	Sint32 i, j;

	CvRect box;
	CvPoint2D32f cp;
	IplImage *cimg;
	CvMat* rot_mat; 

	rot_mat = cvCreateMat(2, 3, CV_32FC1);
	find_biggest_blob(img, &box, Conn4);

	// Calculate blob approximate center
	cp.x = box.x + box.width/2;
	cp.y = box.y + box.height/2;

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
		if (bmed < (obmed - 0.005)) break;
		else obmed = bmed;
	}

	cvReleaseMat(&rot_mat);

	return cur_angle - 0.25;
}
