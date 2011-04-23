#include "spotclear.h"
#include "utils/utils.h"

#include <stdio.h>
#include <math.h>

/* x, y -> start coords for spot measuring
 * in -> image where spot is measured
 * pc -> kind of connection between pixels (4 or 8 edge connection)
 * nval -> this value is substituted to original pixel values in spot, must be different to 0
 * xmin, xmax, ymin, ymax -> pointers to Uint32s, will be set to corners of a rectangle containing the spot
 */
Uint32 size_spot(Uint32 x, Uint32 y, IplImage *in, enum PConn pc, Uint8 nval, Sint32 *xmin, Sint32 *xmax, Sint32 *ymin, Sint32 *ymax);

Uint32 intensity_spot(Uint32 x, Uint32 y, IplImage *in, IplImage *inc, enum PConn pc, Uint8 chan); // Returns the SUM of all intensities in the spot! not the MEDIUM intensity!
void circle_check(IplImage *in, Sint32 xmin, Sint32 xmax, Sint32 ymin, Sint32 ymax, Uint32 *dist);

typedef struct {
    Uint32 x, y;
} Point2D;

#define  POINT_SLOT 4096

/***/

void spot_thin(IplImage *in, Uint16 ssize, float edge_mult, enum PConn pc) {
    IplImage *cin = cvCloneImage(in);

    Sint32 xmin, xmax, ymin, ymax, xsize, ysize;

    Sint32 i, j;
    Uint32 cssize;
    Uint8 *in_dat = in->imageData;
    Uint8 *cin_dat = cin->imageData;

    float lside, sside;

    for (i = 0; i < cin->height; i++)
        for (j = 0; j < cin->width; j++) {
            if (cin_dat[(i * cin->widthStep) + (j * cin->nChannels) + 0] == 0) {
                ymin = ymax = i;
                xmin = xmax = j;

                cssize = size_spot(j, i, cin, pc, 1, &xmin, &xmax, &ymin, &ymax);
                xsize = (xmax - xmin) + 1;
                ysize = (ymax - ymin) + 1;

                lside = MAX(xsize, ysize);
                sside = MIN(xsize, ysize);

                if (cssize <= ssize && lside * edge_mult > sside) {
                    size_spot(j, i, in, pc, 255, NULL, NULL, NULL, NULL);
                }
            }
        }

    cvReleaseImage(&cin);
}

Uint32 intensity_spot(Uint32 x, Uint32 y, IplImage *in, IplImage *inc, enum PConn pc, Uint8 chan) {
    Uint8 *in_dat = in->imageData;
	Uint8 *inc_dat = inc->imageData;
	Uint8 nval = 1;

    Uint8 c;
	Uint32 intval;

    Point2D *p2d = (Point2D*)malloc(sizeof(Point2D) * POINT_SLOT);
    Uint32 max_p2d = POINT_SLOT;
    Uint32 cur_p2d, tot_p2d;

    p2d[0].x = x;
    p2d[0].y = y;

    c = in_dat[(y * in->widthStep) + (x * in->nChannels) + 0];
	intval = in_dat[(y * in->widthStep) + (x * in->nChannels) + chan];

    if (c != 0) return 0;

	in_dat[(y * in->widthStep) + (x * in->nChannels) + 0] = nval;

    tot_p2d = 1;
    Sint32 cur_x, cur_y;
    for (cur_p2d = 0; cur_p2d < tot_p2d; cur_p2d++) {
        cur_x = p2d[cur_p2d].x;
        cur_y = p2d[cur_p2d].y;

        if (pc == Conn8) {
            if ((cur_y + 1 < in->height) && (cur_x + 1 < in->width) && (in_dat[((cur_y + 1) * in->widthStep) + ((cur_x + 1) * in->nChannels) + 0] == 0)) {
                in_dat[((cur_y + 1) * in->widthStep) + ((cur_x + 1) * in->nChannels) + 0] = nval;
				intval += in_dat[((cur_y + 1) * in->widthStep) + ((cur_x + 1) * in->nChannels) + chan];
                p2d[tot_p2d].x = cur_x + 1;
                p2d[tot_p2d].y = cur_y + 1;
                tot_p2d++;

                if(tot_p2d == max_p2d) {
                    max_p2d += POINT_SLOT;
                    p2d = realloc(p2d, sizeof(Point2D) * max_p2d);
                }
            }

            if ((cur_y > 0) && (cur_x > 0) && (in_dat[((cur_y - 1) * in->widthStep) + ((cur_x - 1) * in->nChannels) + 0] == 0)) {
                in_dat[((cur_y - 1) * in->widthStep) + ((cur_x - 1) * in->nChannels) + 0] = nval;
				intval += in_dat[((cur_y - 1) * in->widthStep) + ((cur_x - 1) * in->nChannels) + chan];
                p2d[tot_p2d].x = cur_x - 1;
                p2d[tot_p2d].y = cur_y - 1;
                tot_p2d++;

                if(tot_p2d == max_p2d) {
                    max_p2d += POINT_SLOT;
                    p2d = realloc(p2d, sizeof(Point2D) * max_p2d);
                }
            }

            if ((cur_y + 1 < in->height)  && (cur_x > 0) && (in_dat[((cur_y + 1) * in->widthStep) + ((cur_x - 1) * in->nChannels) + 0] == 0)) {
                in_dat[((cur_y + 1) * in->widthStep) + ((cur_x - 1) * in->nChannels) + 0] = nval;
				intval += in_dat[((cur_y + 1) * in->widthStep) + ((cur_x - 1) * in->nChannels) + chan];
                p2d[tot_p2d].x = cur_x - 1;
                p2d[tot_p2d].y = cur_y + 1;
                tot_p2d++;

                if(tot_p2d == max_p2d) {
                    max_p2d += POINT_SLOT;
                    p2d = realloc(p2d, sizeof(Point2D) * max_p2d);
                }
            }

            if ((cur_y > 0) && (cur_x + 1 < in->width) && (in_dat[((cur_y - 1) * in->widthStep) + ((cur_x + 1) * in->nChannels) + 0] == 0)) {
                in_dat[((cur_y - 1) * in->widthStep) + ((cur_x + 1) * in->nChannels) + 0] = nval;
				intval += in_dat[((cur_y - 1) * in->widthStep) + ((cur_x + 1) * in->nChannels) + chan];
                p2d[tot_p2d].x = cur_x + 1;
                p2d[tot_p2d].y = cur_y - 1;
                tot_p2d++;

                if(tot_p2d == max_p2d) {
                    max_p2d += POINT_SLOT;
                    p2d = realloc(p2d, sizeof(Point2D) * max_p2d);
                }
            }
        }

        if ((cur_y > 0) && (in_dat[((cur_y - 1) * in->widthStep) + (cur_x * in->nChannels) + 0] == 0)) {
            in_dat[((cur_y - 1) * in->widthStep) + (cur_x * in->nChannels) + 0] = nval;
			intval += in_dat[((cur_y - 1) * in->widthStep) + (cur_x * in->nChannels) + chan];
            p2d[tot_p2d].x = cur_x;
            p2d[tot_p2d].y = cur_y - 1;
            tot_p2d++;

            if(tot_p2d == max_p2d) {
                max_p2d += POINT_SLOT;
                p2d = realloc(p2d, sizeof(Point2D) * max_p2d);
            }
        }

        if ((cur_x > 0) && (in_dat[(cur_y * in->widthStep) + ((cur_x - 1) * in->nChannels) + 0] == 0)) {
            in_dat[(cur_y * in->widthStep) + ((cur_x - 1) * in->nChannels) + 0] = nval;
			intval += in_dat[(cur_y * in->widthStep) + ((cur_x - 1) * in->nChannels) + chan];
            p2d[tot_p2d].x = cur_x - 1;
            p2d[tot_p2d].y = cur_y;
            tot_p2d++;

            if(tot_p2d == max_p2d) {
                max_p2d += POINT_SLOT;
                p2d = realloc(p2d, sizeof(Point2D) * max_p2d);
            }
        }

        if ((cur_x + 1 < in->width) && (in_dat[(cur_y * in->widthStep) + ((cur_x + 1) * in->nChannels) + 0] == 0)) {
            in_dat[(cur_y * in->widthStep) + ((cur_x + 1) * in->nChannels) + 0] = nval;
			intval += in_dat[(cur_y * in->widthStep) + ((cur_x + 1) * in->nChannels) + chan];
            p2d[tot_p2d].x = cur_x + 1;
            p2d[tot_p2d].y = cur_y;
            tot_p2d++;

            if(tot_p2d == max_p2d) {
                max_p2d += POINT_SLOT;
                p2d = realloc(p2d, sizeof(Point2D) * max_p2d);
            }
        }

        if ((cur_y + 1 < in->height) && (in_dat[((cur_y + 1) * in->widthStep) + (cur_x * in->nChannels) + 0] == 0)) {
            in_dat[((cur_y + 1) * in->widthStep) + (cur_x * in->nChannels) + 0] = nval;
			intval += in_dat[((cur_y + 1) * in->widthStep) + (cur_x * in->nChannels) + chan];
            p2d[tot_p2d].x = cur_x;
            p2d[tot_p2d].y = cur_y + 1;
            tot_p2d++;

            if(tot_p2d == max_p2d) {
                max_p2d += POINT_SLOT;
                p2d = realloc(p2d, sizeof(Point2D) * max_p2d);
            }
        }

    }

    free(p2d);

    return intval;
}

Uint32 size_spot(Uint32 x, Uint32 y, IplImage *in, enum PConn pc, Uint8 nval, Sint32 *xmin, Sint32 *xmax, Sint32 *ymin, Sint32 *ymax) {
	assert(nval != 0);
    Uint8 *in_dat = in->imageData;
    Uint8 c;

    Point2D *p2d = (Point2D*)malloc(sizeof(Point2D) * POINT_SLOT);
    Uint32 max_p2d = POINT_SLOT;
    Uint32 cur_p2d, tot_p2d;

    p2d[0].x = x;
    p2d[0].y = y;

    c = in_dat[(y * in->widthStep) + (x * in->nChannels) + 0];

    if (c != 0) return 0;

	in_dat[(y * in->widthStep) + (x * in->nChannels) + 0] = nval;

    tot_p2d = 1;
    Sint32 cur_x, cur_y;
    for (cur_p2d = 0; cur_p2d < tot_p2d; cur_p2d++) {
        cur_x = p2d[cur_p2d].x;
        cur_y = p2d[cur_p2d].y;

        if (!(xmin == NULL || xmax == NULL || ymin == NULL || ymax == NULL)) {
            if (cur_x < *xmin) *xmin = cur_x;
            else if (cur_x > *xmax) *xmax = cur_x;

            if (cur_y < *ymin) *ymin = cur_y;
            else if (cur_y > *ymax) *ymax = cur_y;
        }

        if (pc == Conn8) {
            if ((cur_y + 1 < in->height) && (cur_x + 1 < in->width) && (in_dat[((cur_y + 1) * in->widthStep) + ((cur_x + 1) * in->nChannels) + 0] == 0)) {
                in_dat[((cur_y + 1) * in->widthStep) + ((cur_x + 1) * in->nChannels) + 0] = nval;
                p2d[tot_p2d].x = cur_x + 1;
                p2d[tot_p2d].y = cur_y + 1;
                tot_p2d++;

                if(tot_p2d == max_p2d) {
                    max_p2d += POINT_SLOT;
                    p2d = realloc(p2d, sizeof(Point2D) * max_p2d);
                }
            }

            if ((cur_y > 0) && (cur_x > 0) && (in_dat[((cur_y - 1) * in->widthStep) + ((cur_x - 1) * in->nChannels) + 0] == 0)) {
                in_dat[((cur_y - 1) * in->widthStep) + ((cur_x - 1) * in->nChannels) + 0] = nval;
                p2d[tot_p2d].x = cur_x - 1;
                p2d[tot_p2d].y = cur_y - 1;
                tot_p2d++;

                if(tot_p2d == max_p2d) {
                    max_p2d += POINT_SLOT;
                    p2d = realloc(p2d, sizeof(Point2D) * max_p2d);
                }
            }

            if ((cur_y + 1 < in->height)  && (cur_x > 0) && (in_dat[((cur_y + 1) * in->widthStep) + ((cur_x - 1) * in->nChannels) + 0] == 0)) {
                in_dat[((cur_y + 1) * in->widthStep) + ((cur_x - 1) * in->nChannels) + 0] = nval;
                p2d[tot_p2d].x = cur_x - 1;
                p2d[tot_p2d].y = cur_y + 1;
                tot_p2d++;

                if(tot_p2d == max_p2d) {
                    max_p2d += POINT_SLOT;
                    p2d = realloc(p2d, sizeof(Point2D) * max_p2d);
                }
            }

            if ((cur_y > 0) && (cur_x + 1 < in->width) && (in_dat[((cur_y - 1) * in->widthStep) + ((cur_x + 1) * in->nChannels) + 0] == 0)) {
                in_dat[((cur_y - 1) * in->widthStep) + ((cur_x + 1) * in->nChannels) + 0] = nval;
                p2d[tot_p2d].x = cur_x + 1;
                p2d[tot_p2d].y = cur_y - 1;
                tot_p2d++;

                if(tot_p2d == max_p2d) {
                    max_p2d += POINT_SLOT;
                    p2d = realloc(p2d, sizeof(Point2D) * max_p2d);
                }
            }
        }

        if ((cur_y > 0) && (in_dat[((cur_y - 1) * in->widthStep) + (cur_x * in->nChannels) + 0] == 0)) {
            in_dat[((cur_y - 1) * in->widthStep) + (cur_x * in->nChannels) + 0] = nval;
            p2d[tot_p2d].x = cur_x;
            p2d[tot_p2d].y = cur_y - 1;
            tot_p2d++;

            if(tot_p2d == max_p2d) {
                max_p2d += POINT_SLOT;
                p2d = realloc(p2d, sizeof(Point2D) * max_p2d);
            }
        }

        if ((cur_x > 0) && (in_dat[(cur_y * in->widthStep) + ((cur_x - 1) * in->nChannels) + 0] == 0)) {
            in_dat[(cur_y * in->widthStep) + ((cur_x - 1) * in->nChannels) + 0] = nval;
            p2d[tot_p2d].x = cur_x - 1;
            p2d[tot_p2d].y = cur_y;
            tot_p2d++;

            if(tot_p2d == max_p2d) {
                max_p2d += POINT_SLOT;
                p2d = realloc(p2d, sizeof(Point2D) * max_p2d);
            }
        }

        if ((cur_x + 1 < in->width) && (in_dat[(cur_y * in->widthStep) + ((cur_x + 1) * in->nChannels) + 0] == 0)) {
            in_dat[(cur_y * in->widthStep) + ((cur_x + 1) * in->nChannels) + 0] = nval;
            p2d[tot_p2d].x = cur_x + 1;
            p2d[tot_p2d].y = cur_y;
            tot_p2d++;

            if(tot_p2d == max_p2d) {
                max_p2d += POINT_SLOT;
                p2d = realloc(p2d, sizeof(Point2D) * max_p2d);
            }
        }

        if ((cur_y + 1 < in->height) && (in_dat[((cur_y + 1) * in->widthStep) + (cur_x * in->nChannels) + 0] == 0)) {
            in_dat[((cur_y + 1) * in->widthStep) + (cur_x * in->nChannels) + 0] = nval;
            p2d[tot_p2d].x = cur_x;
            p2d[tot_p2d].y = cur_y + 1;
            tot_p2d++;

            if(tot_p2d == max_p2d) {
                max_p2d += POINT_SLOT;
                p2d = realloc(p2d, sizeof(Point2D) * max_p2d);
            }
        }

    }

    free(p2d);

    return tot_p2d;
}

void spot_neighbour_dist(IplImage *in, Uint16 ssize, Uint16 maxdist, enum PConn pc) {
    IplImage *cin = cvCloneImage(in);

    Sint32 xmin, xmax, ymin, ymax, xscenter, yscenter;

    Sint32 i, j;
    Uint32 cssize;
    Uint8 *in_dat = in->imageData;
    Uint8 *cin_dat = cin->imageData;

    Uint32 dist;

    for (i = 0; i < cin->height; i++)
        for (j = 0; j < cin->width; j++) {
            if (cin_dat[(i * cin->widthStep) + (j * cin->nChannels) + 0] == 0) {
                ymin = ymax = i;
                xmin = xmax = j;

                cssize = size_spot(j, i, cin, pc, 1, &xmin, &xmax, &ymin, &ymax);
                xscenter = xmax - (xmax-xmin)/2;
                yscenter = ymax - (ymax-ymin)/2;
                //fprintf(stdout, " This spot size is %ux%u, centered in %ux%u\n", (xmax - xmin) + 1, (ymax - ymin) + 1, xscenter, yscenter);

                dist = maxdist;
                circle_check(in, xmin, xmax, ymin, ymax, &dist);

                if (dist >= maxdist && cssize <= ssize)
                    size_spot(j, i, in, pc, 255, NULL, NULL, NULL, NULL);
            }
        }

    cvReleaseImage(&cin);
}

void remove_spot_size(IplImage *in, Uint16 ssize, enum PConn pc) {
    IplImage *cin = cvCloneImage(in);

    Uint32 xmin, xmax, ymin, ymax;

    Sint32 i, j;
    Uint32 cssize;
    Uint8 *in_dat = in->imageData;
    Uint8 *cin_dat = cin->imageData;
    for (i = 0; i < cin->height; i++)
        for (j = 0; j < cin->width; j++) {
            if (cin_dat[(i * cin->widthStep) + (j * cin->nChannels) + 0] == 0) {
                ymin = ymax = i;
                xmin = xmax = j;

                cssize = size_spot(j, i, cin, pc, 1, &xmin, &xmax, &ymin, &ymax);

                //fprintf(stdout, " This spot size is %u,  %ux%u\n", cssize,  (xmax - xmin) + 1, (ymax - ymin) + 1);

                if(cssize > 0 && cssize <= ssize) {
                    size_spot(j, i, in, pc, 255, NULL, NULL, NULL, NULL);
                }
            }
        }

    cvReleaseImage(&cin);
}

void remove_spot_intensity(IplImage *in, IplImage *gin, Uint16 ssize, Sint16 inc, Uint8 chan, enum PConn pc) {
    Uint8 medium_int = get_medium_intensity(gin, 0);
    Sint32 medium_spot = 0;
    Uint32 tot_spots = 0;
    Sint32 cur_spot_int;

    IplImage *cin = cvCloneImage(in);
    IplImage *cin2 = cvCloneImage(in);

    Sint32 i, j;
    Uint32 cssize;
    for (i = 0; i < cin->height; i++)
        for (j = 0; j < cin->width; j++) {
            if (cin->imageData[(i * cin->widthStep) + (j * cin->nChannels) + 0] == 0) {
                cssize = size_spot(j, i, cin, pc, 1, NULL, NULL, NULL, NULL);

                if(cssize > 0) {
                    tot_spots++;
                    medium_spot += (intensity_spot(j, i, cin2, gin, pc, chan) / cssize);
                }
            }
        }


    cvReleaseImage(&cin);
    cvReleaseImage(&cin2);

    if (!tot_spots) return;

    medium_spot /= tot_spots;

    cin = cvCloneImage(in);
    cin2 = cvCloneImage(in);
    for (i = 0; i < cin->height; i++)
        for (j = 0; j < cin->width; j++) {
            if (cin->imageData[(i * cin->widthStep) + (j * cin->nChannels) + 0] == 0) {
                cssize = size_spot(j, i, cin, pc, 1, NULL, NULL, NULL, NULL);

                if(cssize > 0 && cssize <= ssize) {
                    cur_spot_int = intensity_spot(j, i, cin2, gin, pc, chan) / cssize;

                    if (cur_spot_int + inc >= medium_spot)
                        size_spot(j, i, in, pc, 255, NULL, NULL, NULL, NULL);
                }
            }
        }


    cvReleaseImage(&cin);
    cvReleaseImage(&cin2);
}

void circle_check(IplImage *in, Sint32 xmin, Sint32 xmax, Sint32 ymin, Sint32 ymax, Uint32 *dist) {
    Sint32 xc = xmax - (xmax-xmin)/2;
    Sint32 yc = ymax - (ymax-ymin)/2;

    Sint32 r = 1;
    Sint32 p;
    Uint8 *in_dat = in->imageData;
    Uint8 pixval;

    Sint32 x, y;
    Sint32 sx, sy;

    Sint32 dr = sqrt(pow(xc - (in->width - 1), 2) + pow(yc - (in->height), 2));
    Sint32 dl = sqrt(pow(xc, 2) + pow(yc - (in->height), 2));
    Sint32 ur = sqrt(pow(xc - (in->width - 1), 2) + pow(yc, 2));
    Sint32 ul = sqrt(pow(xc, 2) + pow(yc, 2));
    Sint32 maxd = MAX(dr, dl);
    maxd = MAX(maxd, ur);
    maxd = MAX(maxd, ul);

    //fprintf(stdout, "maxd %d\n", maxd);

    while (r < maxd && r <= (int)*dist) {
        p = 3 - (2 * r);

        x = 0;
        y = r;

        sy = yc - y;
        sx = xc + x;

        if (!(sx < 0 || sy < 0 || sx >= in->width || sy >= in->height)) {
            pixval = in_dat[(sy * in->widthStep) + (sx * in->nChannels) + 0];
            if (pixval != 255 && (sy < ymin || sy > ymax || sx < xmin || sx > xmax)) {
                *dist = r;
                return;
            }
        }

        for (x = 0; x <= y; x++) {
            if (p < 0) {
                p = (p + (4 * x) + 6);
            } else {
                y--;
                p = p + (4 * (x - y) + 10);
            }

            sy = yc - y;
            sx = xc + x;
            if (!(sx < 0 || sy < 0 || sx >= in->width || sy >= in->height)) {
                pixval = in_dat[(sy * in->widthStep) + (sx * in->nChannels) + 0];
                if (pixval != 255 && (sy < ymin || sy > ymax || sx < xmin || sx > xmax)) {
                    *dist = r;
                    return;
                }
            }

            sy = yc - y;
            sx = xc - x;
            if (!(sx < 0 || sy < 0 || sx >= in->width || sy >= in->height)) {
                pixval = in_dat[(sy * in->widthStep) + (sx * in->nChannels) + 0];
                if (pixval != 255 && (sy < ymin || sy > ymax || sx < xmin || sx > xmax)) {
                    *dist = r;
                    return;
                }
            }

            sy = yc + y;
            sx = xc + x;
            if (!(sx < 0 || sy < 0 || sx >= in->width || sy >= in->height)) {
                pixval = in_dat[(sy * in->widthStep) + (sx * in->nChannels) + 0];
                if (pixval != 255 && (sy < ymin || sy > ymax || sx < xmin || sx > xmax)) {
                    *dist = r;
                    return;
                }
            }

            sy = yc + y;
            sx = xc - x;
            if (!(sx < 0 || sy < 0 || sx >= in->width || sy >= in->height)) {
                pixval = in_dat[(sy * in->widthStep) + (sx * in->nChannels) + 0];
                if (pixval != 255 && (sy < ymin || sy > ymax || sx < xmin || sx > xmax)) {
                    *dist = r;
                    return;
                }
            }

            sy = yc - x;
            sx = xc + y;
            if (!(sx < 0 || sy < 0 || sx >= in->width || sy >= in->height)) {
                pixval = in_dat[(sy * in->widthStep) + (sx * in->nChannels) + 0];
                if (pixval != 255 && (sy < ymin || sy > ymax || sx < xmin || sx > xmax)) {
                    *dist = r;
                    return;
                }
            }

            sy = yc - x;
            sx = xc - y;
            if (!(sx < 0 || sy < 0 || sx >= in->width || sy >= in->height)) {
                pixval = in_dat[(sy * in->widthStep) + (sx * in->nChannels) + 0];
                if (pixval != 255 && (sy < ymin || sy > ymax || sx < xmin || sx > xmax)) {
                    *dist = r;
                    return;
                }
            }

            sy = yc + x;
            sx = xc + y;
            if (!(sx < 0 || sy < 0 || sx >= in->width || sy >= in->height)) {
                pixval = in_dat[(sy * in->widthStep) + (sx * in->nChannels) + 0];
                if (pixval != 255 && (sy < ymin || sy > ymax || sx < xmin || sx > xmax)) {
                    *dist = r;
                    return;
                }
            }

            sy = yc + x;
            sx = xc - y;
            if (!(sx < 0 || sy < 0 || sx >= in->width || sy >= in->height)) {
                pixval = in_dat[(sy * in->widthStep) + (sx * in->nChannels) + 0];
                if (pixval != 255 && (sy < ymin || sy > ymax || sx < xmin || sx > xmax)) {
                    *dist = r;
                    return;
                }
            }
        }

        r++;
    }

    *dist = r;
}

