/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#include "play.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <mad.h>
#include <signal.h>

/*
* This is perhaps the simplest example use of the MAD high-level API.
* Standard input is mapped into memory via mmap(), then the high-level API
* is invoked with three callbacks: input, output, and error. The output
* callback converts MAD's high-resolution PCM samples to 16 bits, then
* writes them to standard output in little-endian, stereo-interleaved
* format.
*/

static size_t timeout_read(int fd, char *buf, size_t size, int nsec);

static inline signed scale(register mad_fixed_t sample);

static enum mad_flow input(void *data, struct mad_stream *stream);

static enum mad_flow output(void *data, const struct mad_header *header, struct mad_pcm *pcm);

void play(FILE *fp)
{
	struct stream data;
	struct mad_decoder dec;

	memset(&data, 0, sizeof data);

	data.fp = fp;

	if(NULL == data.fp)
	{
		fprintf(stderr, "stream is null\n");
		return;
	}

	ao_initialize();

	data.driver_id = ao_driver_id("pulse");

	if(-1 == data.driver_id)
	{
		fprintf(stderr, "no driver has been found: %s", strerror(errno));
		return;
	}

	data.fmt.bits = 16;
	data.fmt.rate = 44100;
	data.fmt.channels = 2;
	data.fmt.byte_format = AO_FMT_NATIVE;
	data.device = ao_open_live(data.driver_id, &data.fmt, NULL);

	if(NULL == data.device)
	{
		fprintf(stderr, "unable to open device: %s", strerror(errno));
		return;
	}

	mad_decoder_init(&dec, &data, input, NULL, NULL, output, NULL, NULL);
	mad_decoder_run(&dec, MAD_DECODER_MODE_SYNC);
	mad_decoder_finish(&dec);

	ao_close(data.device);
	ao_shutdown();
}

/*
* This is the input callback. The purpose of this callback is to (re)fill
* the stream buffer which is to be decoded. In this example, an entire file
* has been mapped into memory, so we just call mad_stream_buffer() with the
* address and length of the mapping. When this callback is called a second
* time, we are finished decoding.
*/

#define BUFSIZE 					(1024 << 5)

static enum mad_flow input(void *data, struct mad_stream *stream)
{
	struct stream *ptr = (struct stream *)data;

	static unsigned char buf[BUFSIZE] = { 0 };
	static int nbyte = 0;
	int remnbyte = 0;

	if(feof(ptr->fp))
	{
		/* end of trace */
		return MAD_FLOW_STOP;
	}

	if(stream->next_frame)
	{
		remnbyte = (unsigned)(buf + nbyte - stream->next_frame);
		memcpy(buf, stream->next_frame, remnbyte);
	}
 
	nbyte = timeout_read(fileno(ptr->fp), (char *)(buf + remnbyte), BUFSIZE - remnbyte, 5);

	if(nbyte <= 0)
	{
		if(-1 == nbyte)
		{
			fprintf(stderr, "timeout or occurred an error");
			return MAD_FLOW_STOP;
		}

		return MAD_FLOW_STOP;
	}

	nbyte += remnbyte;

	mad_stream_buffer(stream, buf, nbyte);

	return MAD_FLOW_CONTINUE;
}

/*
* The following utility routine performs simple rounding, clipping, and
* scaling of MAD's high-resolution samples down to 16 bits. It does not
* perform any dithering or noise shaping, which would be recommended to
* obtain any exceptional audio quality. It is therefore not recommended to
* use this routine if high-quality output is desired.
*/

static inline signed scale(register mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if(sample >= MAD_F_ONE)
	{
		sample = MAD_F_ONE - 1;
	}
	else if(sample < -MAD_F_ONE)
	{
		sample = -MAD_F_ONE;
	}

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static enum mad_flow output(void *data, const struct mad_header *header, struct mad_pcm *pcm)
{
	struct stream *ptr = (struct stream *)data;

	register unsigned nsamples = pcm->length;
	unsigned nchannels = pcm->channels, rate = pcm->samplerate;
	const mad_fixed_t *left_ch = pcm->samples[0], *right_ch = pcm->samples[1];

	char *stream, *stream_ptr;

	if((signed)rate != ptr->fmt.rate || (signed)nchannels != ptr->fmt.channels)
	{
		ptr->fmt.rate = rate;
		ptr->fmt.channels = nchannels;

		if(ptr->device != NULL)
		{
			ao_close(ptr->device);
		}
		ptr->device = ao_open_live(ptr->driver_id, &ptr->fmt, NULL);

		if(NULL == ptr->device)
		{
			fprintf(stderr, "unable to open device: %s", strerror(errno));
			return MAD_FLOW_STOP;
		}
	}

	stream_ptr = stream = malloc(pcm->length * (pcm->channels == 2 ? 4 : 2));
	
	register signed sample;

	while(nsamples--)
	{
		sample = scale(*left_ch++);

		*stream_ptr++ = (sample >> 0) & 0xFF;
		*stream_ptr++ = (sample >> 8) & 0xFF;

		if(2 == nchannels)
		{
			sample = scale(*right_ch++);

			*stream_ptr++ = (sample >> 0) & 0xFF;
			*stream_ptr++ = (sample >> 8) & 0xFF;
		}
	}
	ao_play(ptr->device, stream, pcm->length * (pcm->channels == 2 ? 4 : 2));

	free(stream);
	return MAD_FLOW_CONTINUE;
}

static size_t timeout_read(int fd, char *buf, size_t size, int nsec)
{
	fd_set fdset;
	struct timeval tv, *tvp = &tv;

	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);

	if(nsec > 0)
	{
		tv.tv_usec = 0;
		tv.tv_sec = nsec;
	}
	else
	{
		tvp = NULL;
	}

	switch(select(fd + 1, &fdset, NULL, NULL, tvp))
	{
		case -1:
			/* error */
		case 0:
			/* timeout */
			return -1;
	}

	return read(fd, buf, size);
}

