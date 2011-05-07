#ifndef _SPOTCLEAR_HEADER_
#define _SPOTCLEAR_HEADER_

#include "common/defs.h"

enum PConn {Conn4, Conn8};
void remove_spot_intensity(IplImage *in, IplImage *gin, Uint32 minsize, Uint32 maxsize, Sint16 inc, Uint8 chan, enum PConn pc);
void remove_spot_size(IplImage *in, Uint32 minsize, Uint32 maxsize, enum PConn pc);
void remove_spot_neighbour_dist(IplImage *in, Uint32 minsize, Uint32 maxsize, Uint16 maxdist, enum PConn pc);
void remove_spot_thin(IplImage *in, Uint32 minsize, Uint32 maxsize, float edge_mult, enum PConn pc);

Uint32 find_biggest_blob(IplImage *in, CvRect *box, enum PConn pc);

#endif /* _SPOTCLEAR_HEADER_ */
