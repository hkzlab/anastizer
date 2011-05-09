#include "common/defs.h"
#include "common/anast_defs.h"
#include "warptrap/wtrap.h"

// See "globs_anastizer.h"

char dest_file[STR_BUF_SIZE];

WTrap wt[MAX_WTS];
Uint16 wtcode[MAX_WTS];
Uint8 used_wts = DEF_WTS; // Default 

CvMat *invt[MAX_WTS];

IplImage *oimg;
IplImage *prv_img[MAX_WTS];
IplImage *mw_img;

char win_str[64];

