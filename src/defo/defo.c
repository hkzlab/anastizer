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

	Sint32 wrem = img_width % g_width;
	Sint32 hrem = img_height % g_height;
	Sint32 wdist = img_width / g_width;
	Sint32 hdist = img_height / g_height;

	Sint32 i, j;

	// Init the point values in the grid
	for (i = 0; i < g_width; i++)
		for (j = 0; j < g_height; j++) {
			grid->pnt[j * g_width + i].x = wdist * i + wrem / 2;
			grid->pnt[j * g_width + i].y = hdist * j + hrem / 2;
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

	for (i = 1; i < grid->width; i++)
		for (j = 1; j < grid->height; j++) {

			cvCircle(img, cvPoint(grid->pnt[j * grid->width + i].x, grid->pnt[j * grid->width + i].y), 1, col, 2, 8, 0);

			if (i < grid->width - 1)
				cvLine(img, cvPoint(grid->pnt[j * grid->width + i].x, grid->pnt[j * grid->width + i].y), \
						cvPoint(grid->pnt[j * grid->width + (i + 1)].x, grid->pnt[j * grid->width + (i + 1)].y), \
						col, 1, 8, 0);

			if (j < grid->height - 1)
				cvLine(img, cvPoint(grid->pnt[j * grid->width + i].x, grid->pnt[j * grid->width + i].y), \
						cvPoint(grid->pnt[(j + 1) * grid->width + i].x, grid->pnt[(j + 1) * grid->width + i].y), \
						col, 1, 8, 0);
		}
}
