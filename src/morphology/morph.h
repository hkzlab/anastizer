#ifndef _MORPH_HEADER_
#define _MORPH_HEADER_

#include "common/defs.h"

enum Morph { HKZ_ERODE, HKZ_DILATE };

void hkzBaseMorph(const IplImage *src, IplImage *dst, IplConvKernel *se, enum Morph func, Uint32 iterations);
Sint8 hkzMorphComparePics(const IplImage *im1, const IplImage *im2);

#endif /* _MORPH_HEADER_ */
