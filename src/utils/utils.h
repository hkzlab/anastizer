#ifndef _UTILS_HEADER_
#define _UTILS_HEADER_

#include "common/defs.h"

Uint8 get_medium_intensity(IplImage *in, Uint8 chan);
void recalc_img_size(Uint32 *width, Uint32 *height, Uint32 theight);
IplImage *gray_from_colour(IplImage *in, Uint8 chan);

#endif /* _UTILS_HEADER_ */
