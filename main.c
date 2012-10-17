/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#include "trans.h"
#include "play.h"
#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <sys/socket.h>

static char *nskip(const char *str, char ch, int n);

char **translate(const char *text, const char *tolang, const char *srclang, FILE **ttsfile)
{
	char url[1024] = { 0 };
	char tts[1024] = { 0 };

	snprintf(url, 1024, API, text, tolang, tolang, srclang);

	if(ttsfile)
	{
		snprintf(tts, 1024, TTS_API, (int)strlen(text), text, srclang);

		fetch(tts, ttsfile, NULL, NULL);
	}

	return fetch(url, NULL, NULL, NULL);
}

int main(int argc, const char **argv)
{
	char *sl = NULL, *tl = NULL, *text = NULL;
	FILE *tts = NULL;

	register int ch = 0;

	opterr = 0;

	while(-1 != (ch = getopt(argc, (char * const *)argv, "s:t:")))
	{
		switch(ch)
		{
			case 's':
				sl = optarg;
				break;

			case 't':
				tl = optarg;
				break;
		}
	}

	sl = sl ? sl : DEF_SL;
	tl = tl ? tl : DEF_TL;

	if(2 == argc)
	{
		text = (char *)argv[1];
	}
	else
		text = (char *)argv[optind];

	char **resp = translate(text, tl, sl, &tts), *res = NULL;

	if(NULL == resp)
	{
		fprintf(stderr, "failed to get result\n");
		return EXIT_FAILURE;
	}

	res = nskip(*resp, '[', 3);

	if(res != nskip(res, ',', 3))
	{
		nskip(res, ',', 3)[-1] = 0;
	}

	printf("Result: %s\n", res);

	free_response(resp);

	if(tts)
	{
		play(tts);

		shutdown(fileno(tts), SHUT_RDWR);
		fclose(tts);
	}

	return EXIT_SUCCESS;
}

static char *nskip(const char *str, char ch, int n)
{
	char *s = (char *)str;

	for(int i = 0; *s;)
	{
		s = strchr(s, ch);

		if(NULL != s)
		{
			++s;
		}

		++i;

		if(n == i)
		{
			break;
		}
	}

	return *s ? s : NULL;
}
