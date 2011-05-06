#ifndef _UTILS_HEADER_
#define _UTILS_HEADER_

#include "common/defs.h"
#include "warptrap/wtrap.h"

Uint8 get_medium_intensity(IplImage *in, Uint8 chan);
void recalc_img_size(Uint32 *width, Uint32 *height, Uint32 theight);
IplImage *gray_from_colour(IplImage *in, Uint8 chan);
CvMat *build_transf_mat(WTrap *w, CvMat *mm, IplImage *or, IplImage *pw, Uint32 dwidth, Uint32 dheight);
IplImage *return_warped_img(IplImage *oim, CvMat *tm, WTrap *wt, Uint32 dwidth, Uint32 dheight, Sint8 chan);
IplImage *anastize_image(IplImage *wimg, int msize, double mrem); // Apply anastizer filters to an image
#endif /* _UTILS_HEADER_ */
