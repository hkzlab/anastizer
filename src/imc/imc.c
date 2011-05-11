#include "imc.h"

imc_data *allocImcData(Uint32 wts) {
	assert(wts);

	imc_data *dt = (imc_data*) malloc(sizeof(imc_data));
	dt->wt = (WTrap *) malloc (sizeof(WTrap) * wts);
	dt->tot_wts = wts;

	return dt;
}

void freeImcData(imc_data **im) {
	free((*im)->wt);
	free(*im);
	*im = NULL;
}
