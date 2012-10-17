/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <string.h>

#define USERAGENT 			"Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:14.0) Gecko/20100101 Firefox/14.0.1"

#define HEADER_FORMAT 			"%s /%s HTTP/1.1\r\n" 			\
  					"Host: %s\r\n" 				\
					"User-Agent: " USERAGENT "\r\n" 	\
					"Connection: close\r\n"

#define START_WITH(str, need) 		( strncmp((str), (need), strlen((need))) == 0 )
#define END_WITH(str, s) 		( strncmp((str) + strlen((str)) - strlen((s)), (s), strlen((s))) == 0 )

char **fetch(const char *url, FILE **handle, const char *post, const char *type);

void free_response(char **ptr);

char *url_encode(const char *str);

char *url_decode(const char *str);

#endif
