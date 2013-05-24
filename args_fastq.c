/* PANDAseq -- Assemble paired FASTQ Illumina reads and strip the region between amplification primers.
     Copyright (C) 2011-2013  Andre Masella

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */
#define _POSIX_C_SOURCE 2
#include<stdio.h>
#include<stdlib.h>
#include "config.h"
#ifdef HAVE_PTHREAD
#        include<pthread.h>
#endif
#include "pandaseq.h"

struct panda_args_fastq {
	bool fastq;
	bool bzip;
	const char *forward_filename;
	FILE *no_algn_file;
	PandaTagging policy;
	int qualmin;
	const char *reverse_filename;
};

PandaArgsFastq panda_args_fastq_new(
	) {
	PandaArgsFastq data = malloc(sizeof(struct panda_args_fastq));
	data->bzip = false;
	data->forward_filename = NULL;
	data->no_algn_file = NULL;
	data->policy = PANDA_TAG_PRESENT;
	data->qualmin = 33;
	data->reverse_filename = NULL;

	return data;
}

void panda_args_fastq_free(
	PandaArgsFastq data) {
	if (data->no_algn_file != NULL) {
		fclose(data->no_algn_file);
	}
	free(data);
}

bool panda_args_fastq_tweak(
	PandaArgsFastq data,
	char flag,
	const char *argument) {
	switch (flag) {
	case '6':
		data->qualmin = 64;
		return true;
	case 'B':
		data->policy = PANDA_TAG_OPTIONAL;
		return true;
	case 'f':
		data->forward_filename = argument;
		return true;
	case 'j':
		data->bzip = true;
		return true;
	case 'r':
		data->reverse_filename = argument;
		return true;
	case 'u':
		data->no_algn_file = fopen(argument, "w");
		return (data->no_algn_file != NULL);
	default:
		return false;
	}
}

static const panda_tweak_general fastq_phred = { '6', true, NULL, "Use PHRED+64 (CASAVA 1.3-1.7) instead of PHRED+33 (CASAVA 1.8+)." };
static const panda_tweak_general fastq_barcoded = { 'B', true, NULL, "Allow unbarcoded sequences (try this for BADID errors)." };
static const panda_tweak_general fastq_forward = { 'f', false, "forward.fastq", "Input FASTQ file containing forward reads." };
static const panda_tweak_general fastq_bzip = { 'j', true, NULL, "Input files are bzipped." };
static const panda_tweak_general fastq_reverse = { 'r', false, "reverse.fastq", "Input FASTQ file containing reverse reads." };
static const panda_tweak_general fastq_unalign = { 'u', true, "unaligned.txt", "File to write unalignable read pairs." };

const panda_tweak_general *const panda_args_fastq_args[] = {
	&fastq_phred,
	&fastq_barcoded,
	&fastq_forward,
	&fastq_bzip,
	&fastq_reverse,
	&fastq_unalign
};

const size_t panda_args_fastq_args_length = sizeof(panda_args_fastq_args) / sizeof(panda_tweak_general *);

PandaNextSeq panda_args_fastq_opener(
	PandaArgsFastq data,
	PandaLogProxy logger,
	PandaFailAlign *fail,
	void **fail_data,
	PandaDestroy *fail_destroy,
	void **next_data,
	PandaDestroy *next_destroy) {

	if (data->forward_filename == NULL || data->reverse_filename == NULL)
		return NULL;

	if (data->no_algn_file != NULL) {
		*fail = (PandaFailAlign) panda_output_fail;
		*fail_data = data->no_algn_file;
		*fail_destroy = (PandaDestroy) fclose;
		data->no_algn_file = NULL;
	} else {
		*fail = NULL;
		*fail_data = NULL;
		*fail_destroy = NULL;
	}
	return data->bzip ? panda_open_bz2(data->forward_filename, data->reverse_filename, logger, data->qualmin, data->policy, next_data, next_destroy) : panda_open_gz(data->forward_filename, data->reverse_filename, logger, data->qualmin, data->policy, next_data, next_destroy);
}

bool panda_args_fastq_setup(
	PandaArgsFastq data,
	PandaAssembler assembler) {
	/* This doesn't do anything, but it might in future and it's not worth changing the API. */
	return true;
}