#include "handlers_anastizer.h"
#include "windraw_anastizer.h"

#include "common/anast_defs.h"
#include "common/globs_anastizer.h"
#include "common/win_names_anastizer.h"
#include "utils/utils.h"

void prev_mouseHandler(int event, int x, int y, int flags, void *param) {
	Uint16 cur_win = *(Uint16*)param;

	IplImage *gimg;
	IplImage *mimg;
	IplImage *rprev;

	int sres;
	Uint32 nwidth, nheight;

	char tmp_file[STR_BUF_SIZE];
	char winsrc[16];
	tmp_file[0] = '\0';

	sprintf(winsrc, "_WIN%u", cur_win + 1);

	Sint32 qlt_pos = cvGetTrackbarPos(PREV_TRK_QLT, CNTRL_WIN);
	Sint32 msk_pos = cvGetTrackbarPos(PREV_TRK_MSK, CNTRL_WIN);
	Sint32 avr_pos = cvGetTrackbarPos(PREV_TRK_AVR, CNTRL_WIN);
	Sint8 cur_chan = cvGetTrackbarPos(PREV_TRK_BGR, CNTRL_WIN);
	Sint32 agg_pos = cvGetTrackbarPos(PREV_TRK_AGG, CNTRL_WIN);

	msk_pos = msk_pos * 2 + 1;
	msk_pos = msk_pos < 3 ? 3 : msk_pos;
	qlt_pos = (qlt_pos + 1) * 2;

	switch (event) {
	case CV_EVENT_MBUTTONDBLCLK:
	case CV_EVENT_LBUTTONDBLCLK:
	case CV_EVENT_RBUTTONDBLCLK:

		strcat(tmp_file, dest_file);
		strcat(tmp_file, winsrc);

		invt[cur_win] = build_transf_mat(&wt[cur_win], invt[cur_win], oimg, mw_img, prv_img[cur_win]->width * qlt_pos, prv_img[cur_win]->height * qlt_pos);
		
		if (event == CV_EVENT_MBUTTONDBLCLK) { // Save a color version
			gimg = return_warped_img(oimg, invt[cur_win], &wt[cur_win], prv_img[cur_win]->width * qlt_pos, prv_img[cur_win]->height * qlt_pos, -1);

			// Calculate output filename (JPG format)
			strcat(tmp_file, "_WARPED.jpg");
			
			fprintf(stdout, " saving [%ux%u] image to %s ...", gimg->width, gimg->height, tmp_file);
			sres = cvSaveImage(tmp_file, gimg, 0);

			if (sres)
				fprintf(stdout, "OK!\n");
			else
				fprintf(stdout, "Not saved!!!\n");

			fprintf(stdout, " DONE!\n\n");

			cvReleaseImage(&gimg);
			break;

		} else {
			gimg = return_warped_img(oimg, invt[cur_win], &wt[cur_win], prv_img[cur_win]->width * qlt_pos, prv_img[cur_win]->height * qlt_pos, cur_chan);
		}

		mimg = anastize_image(gimg, msk_pos, avr_pos, qlt_pos, agg_pos);

		cvReleaseImage(&gimg);

		// Show it...
		nwidth = mimg->width;
		nheight = mimg->height;
		recalc_img_size(&nwidth, &nheight, PREV_H);
		rprev = cvCreateImage(cvSize(nwidth , nheight), mimg->depth, mimg->nChannels); // Create a resized image
		cvResize(mimg, rprev, CV_INTER_CUBIC); // Resize

		win_str[19] = 49 + cur_win;
		cvShowImage(win_str, rprev);

		if (event == CV_EVENT_RBUTTONDBLCLK) { // or Save it...
			// Calculate output filename (PNG format)
			strcat(tmp_file, "_ANAST.png");

			fprintf(stdout, " saving [%ux%u] image to %s ...", mimg->width, mimg->height, tmp_file);
			sres = cvSaveImage(tmp_file, mimg, 0);

			if (sres)
				fprintf(stdout, "OK!\n");
			else
				fprintf(stdout, "Not saved!!!\n");
		}

		cvReleaseImage(&rprev);
		cvReleaseImage(&mimg);

		fprintf(stdout, " DONE!\n\n");

		break;
	}

	return;
}

void main_mouseHandler(int event, int x, int y, int flags, void *param) {
	static Uint8 lb_down = 0;
	static Uint8 rb_down = 0;
	static Uint8 mb_down = 0;
	static Sint32 curnode = -1;
	static Sint32 cur_wts = -1;

	static int oldx, oldy;
	int xdiff, ydiff;

	Uint32 i;

	switch (event) {
	case CV_EVENT_LBUTTONDOWN:
	case CV_EVENT_RBUTTONDOWN:
	case CV_EVENT_MBUTTONDOWN:
		check_wtrap_point(x, y, wt, &curnode, used_wts);

		cur_wts = curnode / 4;
		curnode %= 4;

		if (event == CV_EVENT_LBUTTONDOWN) lb_down = 1;
		else if (event == CV_EVENT_RBUTTONDOWN) rb_down = 1;
		else if (event == CV_EVENT_MBUTTONDOWN) mb_down = 1;
		oldx = x;
		oldy = y;
		break;
	case CV_EVENT_MBUTTONUP:
	case CV_EVENT_RBUTTONUP:
	case CV_EVENT_LBUTTONUP:
		if (event == CV_EVENT_LBUTTONUP) lb_down = 0;
		else if (event == CV_EVENT_RBUTTONUP) rb_down = 0;
		else if (event == CV_EVENT_MBUTTONUP) mb_down = 0;

		if (curnode >= 0) { // And in this case we should update a preview window...
			win_str[19] = 49 + cur_wts;
			invt[cur_wts] = build_transf_mat(&wt[cur_wts], invt[cur_wts], oimg, mw_img, prv_img[cur_wts]->width, prv_img[cur_wts]->height);
			redraw_preview_win(prv_img[cur_wts], win_str, oimg, invt[cur_wts], &wt[cur_wts]);
		}

		curnode = -1;
		break;
	case CV_EVENT_MOUSEMOVE:
		if (curnode < 0 || (!lb_down && !rb_down && !mb_down)) break;

		xdiff = x - oldx;
		ydiff = y - oldy;

		if (lb_down) {
			if (curnode == 0) {
				wt[cur_wts].a.x += xdiff;
				wt[cur_wts].a.y += ydiff;

				if (wt[cur_wts].a.x > wt[cur_wts].b.x - 10)
					wt[cur_wts].a.x = wt[cur_wts].b.x - 10;

				if (wt[cur_wts].a.x > wt[cur_wts].c.x - 10)
					wt[cur_wts].a.x = wt[cur_wts].c.x - 10;

				if (wt[cur_wts].a.y > wt[cur_wts].d.y - 10)
					wt[cur_wts].a.y = wt[cur_wts].d.y - 10;

				if (wt[cur_wts].a.y > wt[cur_wts].c.y - 10)
					wt[cur_wts].a.y = wt[cur_wts].c.y - 10;

			} else if (curnode == 1) {
				wt[cur_wts].b.x += xdiff;
				wt[cur_wts].b.y += ydiff;

				if (wt[cur_wts].a.x + 10 > wt[cur_wts].b.x)
					wt[cur_wts].b.x = wt[cur_wts].a.x + 10;

				if (wt[cur_wts].d.x + 10 > wt[cur_wts].b.x)
					wt[cur_wts].b.x = wt[cur_wts].d.x + 10;

				if (wt[cur_wts].b.y > wt[cur_wts].c.y - 10)
					wt[cur_wts].b.y = wt[cur_wts].c.y - 10;

				if (wt[cur_wts].b.y > wt[cur_wts].d.y - 10)
					wt[cur_wts].b.y = wt[cur_wts].d.y - 10;

			} else if (curnode == 2) {
				wt[cur_wts].c.x += xdiff;
				wt[cur_wts].c.y += ydiff;

				if (wt[cur_wts].d.x + 10 > wt[cur_wts].c.x)
					wt[cur_wts].c.x = wt[cur_wts].d.x + 10;

				if (wt[cur_wts].a.x + 10 > wt[cur_wts].c.x)
					wt[cur_wts].c.x = wt[cur_wts].a.x + 10;

				if (wt[cur_wts].b.y + 10 > wt[cur_wts].c.y)
					wt[cur_wts].c.y = wt[cur_wts].b.y + 10;

				if (wt[cur_wts].a.y + 10 > wt[cur_wts].c.y)
					wt[cur_wts].c.y = wt[cur_wts].a.y + 10;

			} else if (curnode == 3) {
				wt[cur_wts].d.x += xdiff;
				wt[cur_wts].d.y += ydiff;

				if (wt[cur_wts].d.x > wt[cur_wts].c.x - 10)
					wt[cur_wts].d.x = wt[cur_wts].c.x - 10;

				if (wt[cur_wts].d.x > wt[cur_wts].b.x - 10)
					wt[cur_wts].d.x = wt[cur_wts].b.x - 10;

				if (wt[cur_wts].a.y + 10 > wt[cur_wts].d.y)
					wt[cur_wts].d.y = wt[cur_wts].a.y + 10;

				if (wt[cur_wts].b.y + 10 > wt[cur_wts].d.y)
					wt[cur_wts].d.y = wt[cur_wts].b.y + 10;
			}
		} else if (rb_down) {
			if (wt[cur_wts].a.y + ydiff >= 0 && wt[cur_wts].a.x + xdiff >= 0 && \
			        wt[cur_wts].b.y + ydiff >= 0 && wt[cur_wts].b.x + xdiff < ((IplImage *)param)->width && \
			        wt[cur_wts].c.y + ydiff < ((IplImage *)param)->height && wt[cur_wts].c.x + xdiff < ((IplImage *)param)->width && \
			        wt[cur_wts].d.y + ydiff < ((IplImage *)param)->height && wt[cur_wts].d.x + xdiff >= 0) {

				wt[cur_wts].a.x += xdiff;
				wt[cur_wts].b.x += xdiff;
				wt[cur_wts].c.x += xdiff;
				wt[cur_wts].d.x += xdiff;

				wt[cur_wts].a.y += ydiff;
				wt[cur_wts].b.y += ydiff;
				wt[cur_wts].c.y += ydiff;
				wt[cur_wts].d.y += ydiff;
			}

		} else if (mb_down) {
			if (wt[cur_wts].a.y + ydiff >= 0 && wt[cur_wts].a.x + xdiff >= 0 && \
			        wt[cur_wts].b.y + ydiff >= 0 && wt[cur_wts].b.x + xdiff < ((IplImage *)param)->width && \
			        wt[cur_wts].c.y + ydiff < ((IplImage *)param)->height && wt[cur_wts].c.x + xdiff < ((IplImage *)param)->width && \
			        wt[cur_wts].d.y + ydiff < ((IplImage *)param)->height && wt[cur_wts].d.x + xdiff >= 0) {
				if (curnode == 0) {
					wt[cur_wts].a.x += xdiff;
					wt[cur_wts].a.y += ydiff;

					wt[cur_wts].b.y += ydiff;
					wt[cur_wts].d.x += xdiff;

					if (wt[cur_wts].a.x >= wt[cur_wts].b.x - 10) wt[cur_wts].a.x = wt[cur_wts].b.x - 10;
					if (wt[cur_wts].d.x >= wt[cur_wts].c.x - 10) wt[cur_wts].d.x = wt[cur_wts].c.x - 10;
					if (wt[cur_wts].a.y >= wt[cur_wts].d.y - 10) wt[cur_wts].a.y = wt[cur_wts].d.y - 10;
					if (wt[cur_wts].b.y >= wt[cur_wts].c.y - 10) wt[cur_wts].b.y = wt[cur_wts].c.y - 10;

				} else if (curnode == 1) {
					wt[cur_wts].b.x += xdiff;
					wt[cur_wts].b.y += ydiff;

					wt[cur_wts].a.y += ydiff;
					wt[cur_wts].c.x += xdiff;

					if (wt[cur_wts].a.x >= wt[cur_wts].b.x - 10) wt[cur_wts].b.x = wt[cur_wts].a.x + 10;
					if (wt[cur_wts].d.x >= wt[cur_wts].c.x - 10) wt[cur_wts].c.x = wt[cur_wts].d.x + 10;
					if (wt[cur_wts].a.y >= wt[cur_wts].d.y - 10) wt[cur_wts].a.y = wt[cur_wts].d.y - 10;
					if (wt[cur_wts].b.y >= wt[cur_wts].c.y - 10) wt[cur_wts].b.y = wt[cur_wts].c.y - 10;

				} else if (curnode == 2) {
					wt[cur_wts].c.x += xdiff;
					wt[cur_wts].c.y += ydiff;

					wt[cur_wts].d.y += ydiff;
					wt[cur_wts].b.x += xdiff;

					if (wt[cur_wts].a.x >= wt[cur_wts].b.x - 10) wt[cur_wts].b.x = wt[cur_wts].a.x + 10;
					if (wt[cur_wts].d.x >= wt[cur_wts].c.x - 10) wt[cur_wts].c.x = wt[cur_wts].d.x + 10;
					if (wt[cur_wts].a.y >= wt[cur_wts].d.y - 10) wt[cur_wts].d.y = wt[cur_wts].a.y + 10;
					if (wt[cur_wts].b.y >= wt[cur_wts].c.y - 10) wt[cur_wts].c.y = wt[cur_wts].b.y + 10;

				} else if (curnode == 3) {
					wt[cur_wts].d.x += xdiff;
					wt[cur_wts].d.y += ydiff;

					wt[cur_wts].c.y += ydiff;
					wt[cur_wts].a.x += xdiff;

					if (wt[cur_wts].a.x >= wt[cur_wts].b.x - 10) wt[cur_wts].a.x = wt[cur_wts].b.x - 10;
					if (wt[cur_wts].d.x >= wt[cur_wts].c.x - 10) wt[cur_wts].d.x = wt[cur_wts].c.x - 10;
					if (wt[cur_wts].a.y >= wt[cur_wts].d.y - 10) wt[cur_wts].d.y = wt[cur_wts].a.y + 10;
					if (wt[cur_wts].b.y >= wt[cur_wts].c.y - 10) wt[cur_wts].c.y = wt[cur_wts].b.y + 10;

				}

			}
		}

		oldx = x;
		oldy = y;

		draw_wt_win(MAIN_WIN, mw_img, wt, used_wts);

		break;
	default:
		break;
	}

	return;
}

void cntrl_trk_bgr_handler(int pos) {
	Uint32 i;

	for (i = 0; i < used_wts; i++) {
		win_str[19] = 49 + i;
		invt[i] = build_transf_mat(&wt[i], invt[i], oimg, mw_img, prv_img[i]->width, prv_img[i]->height);
		redraw_preview_win(prv_img[i], win_str, oimg, invt[i], &wt[i]);
	}
}

void cntrl_trk_tmask_handler(int pos) {
	;
}

void cntrl_trk_avr_handler(int pos) {
	;
}

void cntrl_trk_qlt_handler(int pos) {
	Sint32 qlt_pos = cvGetTrackbarPos(PREV_TRK_QLT, CNTRL_WIN);
	Sint32 msk_trkval = (DEFAULT_TMASK * (qlt_pos + 1)) / 2;
	cvSetTrackbarPos(PREV_TRK_MSK, CNTRL_WIN, msk_trkval);
}

void cntrl_trk_agg_handler(int pos) {
	;
}


