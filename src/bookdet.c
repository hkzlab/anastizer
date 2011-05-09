#include "common/defs.h"
#include "common/bookd_defs.h"
#include "utils/utils.h"
#include "spotclear/spotclear.h"

double get_optimum_angle(IplImage *img);

int main(int argc, char *argv[]) {
	Uint32 nwidth, nheight;
	IplImage *oimg, *tmp_img, *smimg, *rot_img;
	Sint32 i, j;

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
	IplImage *timg = whiteThresh(smimg, 120, 25, 1);	
	cvSaveImage("./04testthres.jpg", timg, 0);
	cvErode(timg, timg, NULL, 1);
	cvSaveImage("./05testdil.jpg", timg, 0);

	CvRect box;
	Uint32 spotsize = find_biggest_blob(timg, &box, Conn4);
	remove_spot_size(timg, 1, spotsize - 1, Conn4);
	cvSaveImage("./06testrem.jpg", timg, 0);

	double optangle = get_optimum_angle(timg);
	fprintf(stdout, "opt. rot. angle %f\n", optangle);

	rot_img = cvCloneImage(timg);
	CvPoint2D32f center;
	center.x = box.x + box.width/2;
	center.y = box.y + box.height/2;
	CvMat* rot_mat = cvCreateMat(2, 3, CV_32FC1);
	rot_mat = cv2DRotationMatrix(center, optangle, 1.0, rot_mat);
	cvWarpAffine(timg, rot_img, rot_mat, CV_INTER_NN | CV_WARP_FILL_OUTLIERS, cvScalarAll(255));
	cvReleaseMat(&rot_mat);

	Sint32 fblack, lblack;
	for (j = 0; j < rot_img->width; j++) {
		fblack = -1;
		lblack = -1;

		for (i = 0; i < rot_img->height; i++) {
			if (!rot_img->imageData[(i * rot_img->widthStep) + (j * rot_img->nChannels) + 0]) {
				if (fblack < 0) { fblack = lblack = i; }
				else lblack = i;
			}
		}

		if (fblack > 0)
			cvLine(rot_img, cvPoint(j, fblack), cvPoint(j, lblack), cvScalarAll(0), 1, 8, 0);
	}

	cvSaveImage("./saved.jpg", rot_img, 0);

	Uint32 *rot_proj = (Uint32*)malloc(sizeof(Uint32) * rot_img->width);
	memset(rot_proj, 0, sizeof(Uint32) * rot_img->width);

	for (j = 0; j < rot_img->width; j++)
		for (i = 0; i < rot_img->height; i++) {
			if (rot_img->imageData[(i * rot_img->widthStep) + (j * rot_img->nChannels) + 0])
				rot_proj[i] += 1;
		}

	free(rot_proj);

	cvReleaseImage(&rot_img);
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
		if (bmed < (obmed - 0.005) && box.width > box.height) break;
		else obmed = bmed;
	}

	cvReleaseMat(&rot_mat);

	return cur_angle - 0.25;
}
