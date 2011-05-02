#ifndef _DEFS_HEADER_
#define _DEFS_HEADER_

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

#define MAX_WTS 2 // Max warptraps on screen

#define PREV_H 512 // Preview windows sizes
#define PREV_W 351

#define STR_BUF_SIZE 256 // Temporary dest file string size

#define DEFAULT_RMTH 5 // Default filter options for local thresholding
#define DEFAULT_TMASK 301

#endif /* _DEFS_HEADER_ */
