#include "common/defs.h"
#include "common/anast_defs.h"
#include "warptrap/wtrap.h"
#include "defo/defo.h"

// See "globs_anastizer.h"

char dest_file[STR_BUF_SIZE];

WTrap wt[MAX_WTS];
Uint16 wtcode[MAX_WTS];
Uint8 used_wts = DEF_WTS; // Default used WTS

CvMat *invt[MAX_WTS];

IplImage *oimg;
IplImage *prv_img[MAX_WTS];
IplImage *mw_img;

defo_grid *def_grid; // Default deformation grid, for comparisons
defo_grid *dgrid[MAX_WTS];
Uint8 show_dgrid = 0;

char win_str[64];

