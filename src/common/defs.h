#ifndef _DEFS_HEADER_
#define _DEFS_HEADER_

// Standard headers
#include <stdio.h>
#include <stdlib.h>

// OpenCV headers
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>

typedef unsigned char Uint8;
typedef signed char Sint8;
typedef unsigned short Uint16;
typedef signed short Sint16;
typedef unsigned int Uint32;
typedef signed int Sint32;
typedef unsigned long Uint64;
typedef signed long Sint64;

#define MAX_WTS 4 // Max warptraps on screen
#define DEF_WTS 2 // Default warptraps on screen

#define ORIG_H 2048 // Size of original image
#define PREV_H 600 // Preview windows sizes
#define PREV_W (PREV_H / 1.45)

#define WARP_MULT 2 // Multiplier for resized/anastized images in respect to preview size

#define STR_BUF_SIZE 256 // Temporary dest file string size

#define DEFAULT_RMTH 5 // Default filter options for local thresholding
#define TMASK_MULTIPLIER 10
#define DEFAULT_TMASK ((6 * TMASK_MULTIPLIER * WARP_MULT) + 1)

#endif /* _DEFS_HEADER_ */
