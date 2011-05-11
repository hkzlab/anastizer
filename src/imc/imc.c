#include "imc.h"

#define HEADER "*** IMC HEADER ***"
#define VERSION 0.1

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

int saveImcData(const char *fname, imc_data *dt) {
	assert(fname);
	assert(dt);

	int ret, i;

	FILE *dfile = fopen(fname, "w");
	
	if (dfile == NULL) {
		fprintf(stderr, "saveImcData: Unable to open %s for writing.\n", fname);
		return -1;
	}

	ret = fprintf(dfile, "%s\n", HEADER);
	ret = fprintf(dfile, "%f\n", VERSION);

	ret = fprintf(dfile, "nwts %d\n", dt->tot_wts);
	for (i = 0; i < dt->tot_wts; i++)
		ret = fprintf(dfile, "WT %d %d %d %d %d %d %d\n", dt->wt[i].a.x, dt->wt[i].a.y, dt->wt[i].b.x, dt->wt[i].b.y, dt->wt[i].c.x, dt->wt[i].c.y, dt->wt[i].d.x, dt->wt[i].d.y);

	ret = fprintf(dfile, "qlt %d\n", dt->qlt_trk);
	ret = fprintf(dfile, "bgr %d\n", dt->bgr_trk);
	ret = fprintf(dfile, "avr %d\n", dt->avr_trk);
	ret = fprintf(dfile, "msk %d\n", dt->msk_trk);

	ret = fprintf(dfile, "EOF\n");

	if (ret < 0) {
		fprintf(stderr, "saveImcData: unable to save correctly.\n");
		return -1;
	}

	return 0;
}
