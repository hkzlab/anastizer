#ifndef _SPOTCLEAR_HEADER_
#define _SPOTCLEAR_HEADER_

#include "common/defs.h"

enum PConn {Conn4, Conn8};
void remove_spot_intensity(IplImage *in, IplImage *gin, Uint32 minsize, Uint32 maxsize, Sint16 inc, Uint8 chan, enum PConn pc);
void remove_spot_size(IplImage *in, Uint32 minsize, Uint32 maxsize, enum PConn pc);
void remove_spot_neighbour_dist(IplImage *in, Uint32 minsize, Uint32 maxsize, Uint16 maxdist, enum PConn pc);
void remove_spot_thin(IplImage *in, Uint32 minsize, Uint32 maxsize, float edge_mult, enum PConn pc);

/* x, y -> start coords for spot measuring
 * in -> image where spot is measured
 * pc -> kind of connection between pixels (4 or 8 edge connection)
 * nval -> this value is substituted to original pixel values in spot, must be different than 0
 * xmin, xmax, ymin, ymax -> pointers to Uint32s, will be set to corners of a rectangle containing the spot
 */
Uint32 size_spot(Uint32 x, Uint32 y, IplImage *in, enum PConn pc, Uint8 nval, Sint32 *xmin, Sint32 *xmax, Sint32 *ymin, Sint32 *ymax);

#endif /* _SPOTCLEAR_HEADER_ */
