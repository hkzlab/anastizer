#include "imc.h"

#define HEADER "IMC_HEADER"
#define VERSION 0

imc_data *allocImcData(Uint32 wts) {
	assert(wts);

	imc_data *dt = (imc_data*) malloc(sizeof(imc_data));

	dt->wt = (WTrap *) malloc (sizeof(WTrap) * wts);
	dt->tot_wts = wts;

	dt->qlt_trk = -1;
	dt->bgr_trk = -1;
	dt->avr_trk = -1;
	dt->msk_trk = -1;
	dt->agg_trk = -1;

	return dt;
}

void freeImcData(imc_data **im) {
	assert(im);
	
	if (*im == NULL) return;

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
	ret = fprintf(dfile, "ver %d\n", VERSION);

	ret = fprintf(dfile, "nwts %d\n", dt->tot_wts);
	for (i = 0; i < dt->tot_wts; i++)
		ret = fprintf(dfile, "WT %d %d %d %d %d %d %d %d\n", dt->wt[i].a.x, dt->wt[i].a.y, dt->wt[i].b.x, dt->wt[i].b.y, dt->wt[i].c.x, dt->wt[i].c.y, dt->wt[i].d.x, dt->wt[i].d.y);

	ret = fprintf(dfile, "qlt %d\n", dt->qlt_trk);
	ret = fprintf(dfile, "bgr %d\n", dt->bgr_trk);
	ret = fprintf(dfile, "avr %d\n", dt->avr_trk);
	ret = fprintf(dfile, "msk %d\n", dt->msk_trk);
	ret = fprintf(dfile, "agg %d\n", dt->agg_trk);

	ret = fprintf(dfile, "EOF\n");

	fclose(dfile);

	if (ret < 0) {
		fprintf(stderr, "saveImcData: unable to save correctly.\n");
		return -1;
	}

	return 0;
}

imc_data *loadImcData(const char *fname) {
	char bufstr[64];
	imc_data *dt = NULL;
	int ret, i, nwts;

	Sint32 version; 

	FILE *sfile = fopen(fname, "r");
	if (sfile == NULL) {
		fprintf(stderr, "loadImcData: Unable to open %s for reading.\n", fname);
		return NULL;
	}

	ret = fscanf(sfile, "%63s\n", bufstr);
	
	if(strcmp(bufstr, HEADER) != 0) {
		fprintf(stdout, "bufstr %s\n", bufstr);
		fprintf(stderr, "loadImcData: %s is not in IMC format.\n", fname);
		fclose(sfile);
		return NULL;		
	}

	ret = fscanf(sfile, "ver %d\n", &version);
	ret = fscanf(sfile, "nwts %d\n", &nwts);

	if (nwts < 0 || ret < 0) // We've got a problem...
		return NULL;

	if (version < VERSION) fprintf(stderr, "loadImcData: version mismatch. Expected %d, found %d\n", VERSION, version);

	// Time to alloc some memory
	dt = allocImcData(nwts == 0 ? 1 : nwts); // Always create at least one place for memory, 0 is used when loading default config file
	dt->tot_wts = nwts;

	for (i = 0; i < dt->tot_wts; i++) {
		ret = fscanf(sfile, "WT %d %d %d %d %d %d %d %d\n", &(dt->wt[i].a.x), &(dt->wt[i].a.y), &(dt->wt[i].b.x), &(dt->wt[i].b.y), &(dt->wt[i].c.x), &(dt->wt[i].c.y), &(dt->wt[i].d.x), &(dt->wt[i].d.y));
		
		if (ret < 0) {
			freeImcData(&dt);
			return NULL;
		}
	}

	ret = fscanf(sfile, "qlt %d\n", &(dt->qlt_trk));
	ret = fscanf(sfile, "bgr %d\n", &(dt->bgr_trk));
	ret = fscanf(sfile, "avr %d\n", &(dt->avr_trk));
	ret = fscanf(sfile, "msk %d\n", &(dt->msk_trk));
	ret = fscanf(sfile, "agg %d\n", &(dt->agg_trk));

	ret = fscanf(sfile, "%10s\n", bufstr);

	if(strcmp(bufstr, "EOF") != 0 || ret < 0) {
		fprintf(stderr, "loadImcData: file %s got truncated.\n", fname);
		freeImcData(&dt);
		return NULL;		
	}

	fclose(sfile);

	return dt;
}
