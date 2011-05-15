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

void warpDefoMaps(IplImage *mapx, IplImage *mapy, defo_grid *dgrid, defo_grid *ogrid) {
	assert(mapx);
	assert(mapy);
	assert(dgrid);
	assert(ogrid);

	assert(mapx->depth == IPL_DEPTH_32F && mapx->nChannels == 1);
	assert(mapy->depth == IPL_DEPTH_32F && mapy->nChannels == 1);

	IplImage *mapxc, *mapyc, *tmpi;

	CvMemStorage *ms = cvCreateMemStorage(0);
	CvSeq *contour = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), ms);

	CvMat *mm = cvCreateMat(3, 3, CV_32FC1); // Warp matrix

	mapxc = cvCloneImage(mapx);
	mapyc = cvCloneImage(mapy);

	CvPoint2D32f wsrc[4]; // Source and destination points for warping
	CvPoint2D32f wdst[4];

	CvRect roirect, tmpsize;
	Sint32 i, j, maxx, minx, maxy, miny;
	CvPoint a, b, c, d;
	// Traverse each rectangle in the original grid
	for (i = 0; i < ogrid->width - 1; i++)
		for (j = 0; j < ogrid->height - 1; j++) {
			roirect.x = ogrid->pnt[j * ogrid->width + i].x;
			roirect.y = ogrid->pnt[j * ogrid->width + i].y;
			roirect.width = (ogrid->pnt[j * ogrid->width + i].x - ogrid->pnt[j * ogrid->width + (i + 1)].x) + 1;
			roirect.height = (ogrid->pnt[j * ogrid->width + i].y - ogrid->pnt[(j + 1) * ogrid->width + i].y) + 1;

			// Set ROIs around current grid area
			cvSetImageROI(mapxc, roirect);
			cvSetImageROI(mapyc, roirect);

			a.x = dgrid->pnt[j * dgrid->width + i].x;
			a.y = dgrid->pnt[j * dgrid->width + i].y;
			b.x = dgrid->pnt[j * dgrid->width + (i + 1)].x;
			b.y = dgrid->pnt[j * dgrid->width + (i + 1)].y;
			c.x = dgrid->pnt[(j + 1) * dgrid->width + (i + 1)].x;
			c.y = dgrid->pnt[(j + 1) * dgrid->width + (i + 1)].y;
			d.x = dgrid->pnt[(j + 1) * dgrid->width + i].x;
			d.y = dgrid->pnt[(j + 1) * dgrid->width + i].y;

			cvSeqPush(contour, &a);
			cvSeqPush(contour, &b);
			cvSeqPush(contour, &c);
			cvSeqPush(contour, &d);

			tmpsize = cvBoundingRect(contour, 0); // Calculate destination bounding rect

			cvClearSeq(contour);

			// Warp points FIXME: maybe these need to be absolute, and not relativeto ROI
			wsrc[0].x = wsrc[0].y = 0; 
			wsrc[1].x = roirect.width - 1; wsrc[1].y = 0;
			wsrc[2].x = roirect.width - 1; wsrc[2].y = roirect.height - 1;
			wsrc[3].x = 0; wsrc[3].y = roirect.height - 1;

			wdst[0].x = a.x - tmpsize.x; wdst[0].y = a.y - tmpsize.y;
			wdst[1].x = b.x - tmpsize.x; wdst[1].y = b.y - tmpsize.y;
			wdst[2].x = c.x - tmpsize.x; wdst[2].y = c.y - tmpsize.y;
			wdst[3].x = d.x - tmpsize.x; wdst[3].y = d.y - tmpsize.y;

			mm = cvGetPerspectiveTransform(wdst, wsrc, mm); // Get warpmat

			tmpi = cvCreateImage(cvSize(tmpsize.width, tmpsize.height), IPL_DEPTH_32F, 1);

			cvWarpPerspective(mapx, tmpi, mm, CV_INTER_LINEAR | CV_WARP_FILL_OUTLIERS, cvScalarAll(0));
			// TODO: save warped matrix

			cvWarpPerspective(mapy, tmpi, mm, CV_INTER_LINEAR | CV_WARP_FILL_OUTLIERS, cvScalarAll(0));
			// TODO: save warped matrix

			cvReleaseImage(&tmpi);

			cvResetImageROI(mapxc);
			cvResetImageROI(mapyc);
		}

	cvClearMemStorage(ms);
	cvReleaseMemStorage(&ms);

	cvReleaseImage(&mapxc);
	cvReleaseImage(&mapyc);

	cvReleaseMat(&mm);

}
