#ifndef _MORPH_HEADER_
#define _MORPH_HEADER_

#include "common/defs.h"

enum Morph { HKZ_ERODE, HKZ_DILATE };

void hkzBaseMorph(const IplImage *src, IplImage *dst, IplConvKernel *se, enum Morph func, Uint32 iterations);
Sint8 hkzMorphComparePics(const IplImage *im1, const IplImage *im2);
void hkzMorphThin(const IplImage *src, IplImage *dst, IplConvKernel *se);
void hkzBinPicSub(const IplImage *im1, IplImage *im2);
void hkzMorphFullThin(const IplImage *src, IplImage *dst);
void hkzMorphOpen(const IplImage *src, IplImage *dst, IplConvKernel *se);
void hkzMorphClose(const IplImage *src, IplImage *dst, IplConvKernel *se);

#endif /* _MORPH_HEADER_ */
