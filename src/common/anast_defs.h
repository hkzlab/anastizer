#ifndef _ANAST_DEFS_HEADER_
#define _ANAST_DEFS_HEADER_

#define MAX_WTS 4 // Max warptraps on screen
#define DEF_WTS 2 // Default warptraps on screen

#define ORIG_H 2048 // Size of original image
#define PREV_H 512 // Preview windows sizes
#define PREV_W (PREV_H / 1.45)

#define WARP_MULT 2 // Multiplier for resized/anastized images in respect to preview size

#define STR_BUF_SIZE 256 // Temporary dest file string size

#define DEFAULT_RMTH 20 // Default filter options for local thresholding
#define TMASK_MULTIPLIER 10
#define DEFAULT_TMASK ((16 * TMASK_MULTIPLIER * WARP_MULT) + 1)

#endif /* _ANAST_DEFS_HEADER_ */
