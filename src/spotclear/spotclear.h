#ifndef _SPOTCLEAR_HEADER_
#define _SPOTCLEAR_HEADER_

#include "common/defs.h"

enum PConn {Conn4, Conn8};
void remove_spot_intensity(IplImage *in, IplImage *gin, Uint16 ssize, Sint16 inc, Uint8 chan, enum PConn pc);
void remove_spot_size(IplImage *in, Uint16 ssize, enum PConn pc);
void spot_neighbour_dist(IplImage *in, Uint16 ssize, Uint16 maxdist, enum PConn pc);
void spot_thin(IplImage *in, Uint16 ssize, float edge_mult, enum PConn pc);

#endif /* _SPOTCLEAR_HEADER_ */
