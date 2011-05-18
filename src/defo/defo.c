#include "defo.h"

defo_grid *allocDefoGrid(Sint32 width, Sint32 height) {
	assert(width > 4 && height > 4);

	defo_grid *d = (defo_grid*) malloc(sizeof(defo_grid));
	d->width = width;
	d->height = height;

	d->pnt = (CvPoint2D32f*)malloc(sizeof(CvPoint2D32f) * width * height);

	return d;
}

void freeDefoGrid(defo_grid **dg) {
	assert(*dg);

	free((*dg)->pnt);
	free(*dg);

	*dg = NULL;
}

void initDefoGrid(IplImage *img, defo_grid *grid) {
	assert(img);
	assert(grid);

	Sint32 img_width = img->width;
	Sint32 img_height = img->height;
	Sint32 g_width = grid->width;
	Sint32 g_height = grid->height;

	Sint32 wrem = img_width % (g_width - 1);
	Sint32 hrem = img_height % (g_height - 1);
	Sint32 wdist = img_width / (g_width - 1);
	Sint32 hdist = img_height / (g_height - 1);

	Sint32 i, j;

	// Init the point values in the grid
	for (i = 0; i < g_width; i++)
		for (j = 0; j < g_height; j++) {
			grid->pnt[j * g_width + i].x = wdist * i + (wrem / 2);
			grid->pnt[j * g_width + i].y = hdist * j + (hrem / 2);
		}

}

#define MAXDIST 4
defo_point *findDefoPoint(Sint32 xclick, Sint32 yclick, defo_grid *grid) {
	assert(xclick >= 0 && yclick >= 0);
	assert(grid);

	defo_point *dp = NULL;

	Sint32 p_x, p_y; // Here i'll keep the saved points coords in grid
	Sint32 i, j, curdist;
	Sint32 dist = -1;
	p_x = p_y = -1;

	for (i = 0; i < grid->width; i++)
		for (j = 0; j < grid->height; j++) {
			if (abs(xclick - grid->pnt[j * grid->width + i].x) < MAXDIST &&  abs(yclick - grid->pnt[j * grid->width + i].y) < MAXDIST) { // We're near enough...
				curdist = abs(xclick - grid->pnt[j * grid->width + i].x) + abs(yclick - grid->pnt[j * grid->width + i].y);
				if (curdist < dist || dist < 0) {
					dist = curdist;
					p_x = i;
					p_y = j;
				}
			}
		}

	if (dist >= 0) {
		dp = (defo_point*)malloc(sizeof(defo_point));
		dp->px = p_x;
		dp->py = p_y;
		dp->pnt = &(grid->pnt[p_y * grid->width + p_x]);
	}

	return dp;
}

void moveDefoPoint(Sint32 xdiff, Sint32 ydiff, defo_point *dp, defo_grid *grid) {
	assert(dp);
	assert(grid);

	// First of all, if the point is on a border, it cannot be moved at all.
	if (dp->px == 0 || dp->px == grid->width - 1 || dp->py == 0 || dp->py == grid->height - 1) return;

	// Now, for all the remaining points we can identify 8 other points in their neighbourhood, we must make sure
	// we don't move past those points coords

	CvMemStorage *ms = cvCreateMemStorage(0);
	CvSeq *contour = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), ms);

	CvPoint pul, pu, pur, pr, pdr, pd, pdl, pl;
	pul = cvPoint(grid->pnt[(dp->py - 1) * grid->width + (dp->px - 1)].x, grid->pnt[(dp->py - 1) * grid->width + (dp->px - 1)].y);
	pu = cvPoint(grid->pnt[(dp->py - 1) * grid->width + dp->px].x, grid->pnt[(dp->py - 1) * grid->width + dp->px].y);
	pur = cvPoint(grid->pnt[(dp->py - 1) * grid->width + (dp->px + 1)].x, grid->pnt[(dp->py - 1) * grid->width + (dp->px + 1)].y);
	pr = cvPoint(grid->pnt[dp->py * grid->width + (dp->px + 1)].x, grid->pnt[dp->py * grid->width + (dp->px + 1)].y);
	pdr = cvPoint(grid->pnt[(dp->py + 1) * grid->width + (dp->px + 1)].x, grid->pnt[(dp->py + 1) * grid->width + (dp->px + 1)].y);
	pd = cvPoint(grid->pnt[(dp->py + 1) * grid->width + dp->px].x, grid->pnt[(dp->py + 1) * grid->width + dp->px].y);
	pdl = cvPoint(grid->pnt[(dp->py + 1) * grid->width + (dp->px - 1)].x, grid->pnt[(dp->py + 1) * grid->width + (dp->px - 1)].y);
	pl = cvPoint(grid->pnt[dp->py * grid->width + (dp->px - 1)].x, grid->pnt[dp->py * grid->width + (dp->px - 1)].y);

	cvSeqPush(contour, &pul);
	cvSeqPush(contour, &pu);
	cvSeqPush(contour, &pur);
	cvSeqPush(contour, &pr);
	cvSeqPush(contour, &pdr);
	cvSeqPush(contour, &pd);
	cvSeqPush(contour, &pdl);
	cvSeqPush(contour, &pl);

	// See cvPointPolygonTest
	double inside = cvPointPolygonTest(contour, cvPoint2D32f(dp->pnt->x + xdiff, dp->pnt->y + ydiff), 0);

	cvClearSeq(contour);
	cvClearMemStorage(ms);
	cvReleaseMemStorage(&ms);

	// Destination is inside, we can move it
	if (inside > 0) { dp->pnt->x += xdiff; dp->pnt->y += ydiff; }
}

void drawDefoGrid(IplImage *img, defo_grid *grid, CvScalar col) {
	assert(img);
	assert(grid);

	Sint32 i, j;

	for (i = 0; i < grid->width; i++)
		for (j = 0; j < grid->height; j++) {

			if (i > 0 && j > 0 && i < grid->width - 1 && j < grid->height - 1)
				cvCircle(img, cvPoint(grid->pnt[j * grid->width + i].x, grid->pnt[j * grid->width + i].y), 1, col, 2, 8, 0);
			
			if (i + 1 < grid->width)
				cvLine(img, cvPoint(grid->pnt[j * grid->width + i].x, grid->pnt[j * grid->width + i].y), \
						cvPoint(grid->pnt[j * grid->width + (i + 1)].x, grid->pnt[j * grid->width + (i + 1)].y), \
						col, 1, 8, 0);

			if (j + 1 < grid->height)
				cvLine(img, cvPoint(grid->pnt[j * grid->width + i].x, grid->pnt[j * grid->width + i].y), \
						cvPoint(grid->pnt[(j + 1) * grid->width + i].x, grid->pnt[(j + 1) * grid->width + i].y), \
						col, 1, 8, 0);
		}
}

IplImage *warpDefoImg(IplImage *img, defo_grid *dgrid, defo_grid *ogrid) {
	assert(img);
	assert(dgrid);
	assert(ogrid);

	IplImage *cimg = cvCloneImage(img);
	IplImage *temp_img = NULL;

	CvMat *wmat = cvCreateMat(3, 3, CV_32FC1);
	CvPoint2D32f srcp[4], dstp[4], tmpp[4];

	Sint32 i, j;

	tmpp[0].x = tmpp[3].x = 0;
	tmpp[0].y = tmpp[1].y = 0;

	// Go through all the grid rectangles
	for (i = 0; i < ogrid->width - 1; i++)
		for (j = 0; j < ogrid->height - 1; j++) {
			// Obtain the coords of original points
			srcp[0] = ogrid->pnt[j * ogrid->width + i];
			srcp[1] = ogrid->pnt[j * ogrid->width + (i + 1)];
			srcp[2] = ogrid->pnt[(j + 1) * ogrid->width + (i + 1)];
			srcp[3] = ogrid->pnt[(j + 1) * ogrid->width + i];

			dstp[0] = dgrid->pnt[j * dgrid->width + i];
			dstp[1] = dgrid->pnt[j * dgrid->width + (i + 1)];
			dstp[2] = dgrid->pnt[(j + 1) * dgrid->width + (i + 1)];
			dstp[3] = dgrid->pnt[(j + 1) * dgrid->width + i];

			if (srcp[0].x == dstp[0].x && srcp[1].x == dstp[1].x && srcp[2].x == dstp[2].x && srcp[3].x == dstp[3].x && \
				srcp[0].y == dstp[0].y && srcp[1].y == dstp[1].y && srcp[2].y == dstp[2].y && srcp[3].y == dstp[3].y) continue;

			cvSetImageROI(img, cvRect(srcp[0].x, srcp[0].y, (srcp[2].x - srcp[0].x) + 1, (srcp[2].y - srcp[0].y) + 1));
			temp_img = cvCreateImage(cvGetSize(img), img->depth, img->nChannels); // Create a subimage the same size of the ROI
			cvCopy(img, temp_img, NULL);
			cvResetImageROI(img); // Reset the ROI

			tmpp[2].y = tmpp[3].y = (srcp[2].y - srcp[0].y) + 1;
			tmpp[1].x = tmpp[2].x = (srcp[2].x - srcp[0].x) + 1;

			wmat = cvGetPerspectiveTransform(tmpp, dstp, wmat);

			cvWarpPerspective(temp_img, cimg, wmat, CV_INTER_CUBIC, cvScalarAll(0));

			cvReleaseImage(&temp_img);
		}

	cvReleaseMat(&wmat);

	return cimg;
}
