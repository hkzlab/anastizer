#ifndef _HANDLERS_ANASTIZER_HEADER_
#define _HANDLERS_ANASTIZER_HEADER_

#include "common/defs.h"

void main_mouseHandler(int event, int x, int y, int flags, void *param);
void prev_mouseHandler(int event, int x, int y, int flags, void *param);
void cntrl_trk_bgr_handler(int pos);
void cntrl_trk_tmask_handler(int pos);
void cntrl_trk_avr_handler(int pos);
void cntrl_trk_qlt_handler(int pos);
void cntrl_trk_agg_handler(int pos);

#endif /* _HANDLERS_ANASTIZER_HEADER_ */
