#include "common/defs.h"
#include "warptrap/wtrap.h"

// See "globs.h"

Uint16 tmask_size = DEFAULT_TMASK;
Uint16 tmask_avr = DEFAULT_RMTH;

char dest_file[STR_BUF_SIZE];

WTrap wt[MAX_WTS];
Uint16 wtcode[MAX_WTS];
Uint8 used_wts = 2;

CvMat *invt[MAX_WTS];

IplImage *oimg;
IplImage *prv_img[MAX_WTS];
IplImage *mw_img;

char win_str[64];

