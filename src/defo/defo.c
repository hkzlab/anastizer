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
	assert(mapx->width == mapy->width && mapx->height == mapy->height);

	CvMemStorage *ms = cvCreateMemStorage(0);
	CvSeq *contour = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), ms); // Memory storage for contour checking

	CvPoint p1, p2, p3, p4; // These will be used for contour checking
	CvPoint pmax, pmin;

	float ax, bx, cx, dx; // linear interpolation coefficients
	float ay, by, cy, dy;

	float vx1, vx2, vx3, vx4; // Values at vertices, for interpolation
	float vy1, vy2, vy3, vy4;

	Sint32 i, j, k, n;

	double inside;

	// Initialize the maps
	for (i = 0; i < mapx->height; i++)
		for (j = 0; j < mapx->width; j++) {
			cvSetReal2D(mapx, i, j, j);
			cvSetReal2D(mapy, i, j, i);
		}

	// Go through all the grid rectangles
	for (i = 0; i < ogrid->width - 1; i++)
		for (j = 0; j < ogrid->height - 1; j++) {
			vx1 = ogrid->pnt[j * ogrid->width + i].x;
			vx2 = ogrid->pnt[j * ogrid->width + (i + 1)].x;
			vx3 = ogrid->pnt[(j + 1) * ogrid->width + (i + 1)].x;
			vx4 = ogrid->pnt[(j + 1) * ogrid->width + i].x;

			// Calculate the coefficients
			ax = vx2 - vx1;
			bx = vx4 - vx1;
			cx = vx3 + vx1 - vx4 - vx2;
			dx = vx1;

			vy1 = ogrid->pnt[j * ogrid->width + i].y;
			vy2 = ogrid->pnt[j * ogrid->width + (i + 1)].y;
			vy3 = ogrid->pnt[(j + 1) * ogrid->width + (i + 1)].y;
			vy4 = ogrid->pnt[(j + 1) * ogrid->width + i].y;

			// Calculate the coefficients
			ay = vy2 - vy1;
			by = vy4 - vy1;
			cy = vy3 + vy1 - vy4 - vy2;
			dy = vy1;

			// Save point coordinates for warped grid rectangle
			p1.x = dgrid->pnt[j * dgrid->width + i].x;
			p1.y = dgrid->pnt[j * dgrid->width + i].y;
			p2.x = dgrid->pnt[j * dgrid->width + (i + 1)].x;
			p2.y = dgrid->pnt[j * dgrid->width + (i + 1)].y;
			p3.x = dgrid->pnt[(j + 1) * dgrid->width + (i + 1)].x;
			p3.y = dgrid->pnt[(j + 1) * dgrid->width + (i + 1)].y;
			p4.x = dgrid->pnt[(j + 1) * dgrid->width + i].x;
			p4.y = dgrid->pnt[(j + 1) * dgrid->width + i].y;

			pmax.x = MAX(p1.x, p2.x); pmax.x = MAX(pmax.x, p3.x); pmax.x = MAX(pmax.x, p4.x);
			pmax.y = MAX(p1.y, p2.y); pmax.y = MAX(pmax.y, p3.y); pmax.y = MAX(pmax.y, p4.y);

			pmin.x = MIN(p1.x, p2.x); pmin.x = MIN(pmin.x, p3.x); pmin.x = MIN(pmin.x, p4.x);
			pmin.y = MIN(p1.y, p2.y); pmin.y = MIN(pmin.y, p3.y); pmin.y = MIN(pmin.y, p4.y);

			for (k = pmin.x; k <= pmax.x; k++)
				for (n = pmin.y; n <= pmax.y; n++) {
					inside = cvPointPolygonTest(contour, cvPoint2D32f(k, n), 0); // Check if we're inside the warped rect
					if (inside >= 0) {
						// Save the interpolated points in the maps
						cvSetReal2D(mapx, k, n, ax * k + bx * n + cx * n * k + dx);
						cvSetReal2D(mapy, k, n, ay * k + by * n + cy * n * k + dy);
					}
				}

			cvClearSeq(contour);
		}


	cvClearMemStorage(ms);
	cvReleaseMemStorage(&ms);

}
