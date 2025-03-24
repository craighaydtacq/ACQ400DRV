/*
 * meanblock.cpp : profile calculating mean over NSAMPLES
 *
 *  Created on: 13 Feb 2025
 *      Author: pgm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int nchan = 192;
int nsam = 100;
int iter = 1;
int dummy_run = 0;		/* to eliminate overheads */
int transpose = 0;              /* channelize .. the main overhead in Judgement, for ex. */

short * data;
short * channels;
long * result;

void get_data(void)
{
	data = new short[nsam*nchan];
	result = new long[nchan];
	memset(result, 0, nchan*sizeof(long));
	if (transpose){
		channels = new short[nchan*nsam];
	}
}

void free_data(void)
{
	delete [] data;
	delete [] result;
	if (transpose){
		delete [] channels;
	}
}

void calculate_mean() {
	int is, ic;

	for (is = 0; is < nsam; ++is){
		const short* row = data+is*nchan;
		for (ic = 0; ic < nchan; ++ic){
			result[ic] += row[ic];
		}
	}
	for (ic = 0; ic < nchan; ++ic){
		result[ic] /= is;
	}
}

void calculate_mean_and_transpose() {
	int is, ic;

	for (is = 0; is < nsam; ++is){
		const short* const row = data+is*nchan;
		for (ic = 0; ic < nchan; ++ic){
			result[ic] += row[ic];
			channels[ic*nsam + is] = row[ic];
		}
	}
	for (ic = 0; ic < nchan; ++ic){
		result[ic] /= is;
	}
}

void calculate_mean_and_copy() {
	int is, ic;

	for (is = 0; is < nsam; ++is){
		const short* const row = data+is*nchan;
		short* const rowto = channels+is*nchan;
		for (ic = 0; ic < nchan; ++ic){
			result[ic] += row[ic];
			rowto[ic] = row[ic];
		}
	}
	for (ic = 0; ic < nchan; ++ic){
		result[ic] /= is;
	}
}

int main(int argc, const char* argv[])
{
	if (argc > 1) nchan = atoi(argv[1]);
	if (argc > 2) nsam = atoi(argv[2]);
	if (argc > 3) iter = atoi(argv[3]);
	if (argc > 4) dummy_run = atoi(argv[4]);
	if (argc > 5) transpose = atoi(argv[5]);

	void (*calc)(void) =
			transpose==2? calculate_mean_and_copy:
			transpose==1? calculate_mean_and_transpose:
				      calculate_mean;

	for (int it = 0; it < iter; ++it){
		get_data();
		if (!dummy_run){
			calc();
		}
		free_data();
	}
}


