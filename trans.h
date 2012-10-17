/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#ifndef TRANS_H
#define TRANS_H

#define API 						"http://translate.google.cn/translate_a/t" 	\
  							"?text=%s" 					\
							"&otf=1&tsel=0" 				\
							"&hl=%s" 					\
							"&mulitres=1&ie=UTF-8&oe=UTF-8" 		\
							"&tl=%s" 					\
							"&client=t" 					\
							"&sl=%s"  					\
							"&sc=1&ssel=0"

#define TTS_API 					"http://translate.google.cn/translate_tts" 	\
  							"?total=1&textlen=%d"  				\
							"&idx=0&q=%s" 					\
							"&tl=%s" 					\
							"&prev=input&ie=UTF-8"

#define DEF_HL 						"zh-CN"
#define DEF_TL 						"zh-CN"
#define DEF_SL 						"en"

#include <stdio.h>

char **translate(const char *text, const char *tolang, const char *srclang, FILE **ttsfile);

#endif
