#ifndef _IMC_HEADER_
#define _IMC_HEADER_

#include "common/defs.h"
#include "warptrap/wtrap.h"

typedef struct {
	int qlt_trk;
	int bgr_trk;
	int avr_trk;
	int msk_trk;
	int agg_trk;
	int rat_trk;

	int tot_wts;
	WTrap *wt;
} imc_data;

imc_data *allocImcData(Uint32 wts);
void freeImcData(imc_data **im);

int saveImcData(const char *fname, imc_data *dt);
imc_data *loadImcData(const char *fname);

#endif /* _IMC_HEADER_ */
