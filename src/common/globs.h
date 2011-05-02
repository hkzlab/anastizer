#ifndef _GLOBS_HEADER_
#define _GLOBS_HEADER_

#include "common/defs.h"
#include "warptrap/wtrap.h"

extern Uint16 tmask_size;
extern Uint16 tmask_avr;

extern char dest_file[STR_BUF_SIZE];

extern WTrap wt[MAX_WTS];
extern Uint16 wtcode[MAX_WTS];

extern CvMat *invt[MAX_WTS];

extern IplImage *oimg; // Original image;
extern IplImage *prv_img[MAX_WTS];
extern IplImage *mw_img;
extern char win_str[64];

#endif /* _GLOBS_HEADER_ */
