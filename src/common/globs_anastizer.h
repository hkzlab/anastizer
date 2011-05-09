#ifndef _GLOBS_ANASTIZER_HEADER_
#define _GLOBS_ANASTIZER_HEADER_

#include "common/defs.h"
#include "warptrap/wtrap.h"

// These are global vars required by OpenCV gui handler's callbacks

extern char dest_file[STR_BUF_SIZE]; // File destination path buffer

extern WTrap wt[MAX_WTS]; // WarpTraps data
extern Uint16 wtcode[MAX_WTS];
extern Uint8 used_wts; // Currently used warptraps

extern CvMat *invt[MAX_WTS]; // Inversion matrices for screen warping

extern IplImage *oimg; // Original image;
extern IplImage *prv_img[MAX_WTS]; // Preview images
extern IplImage *mw_img; // Main window resized image

extern char win_str[64]; // Window title string for preview windows

#endif /* _GLOBS_ANASTIZER_HEADER_ */
