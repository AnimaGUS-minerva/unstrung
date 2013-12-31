/*
 * Copyright (C) 2009-2013 Michael Richardson <mcr@sandelman.ca>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include <sys/time.h>
#include <time.h>
}

#include "debug.h"

void rpl_debug::logv_more(const char *fmt, va_list vargs)
{
    if(file == NULL) return;
    vfprintf(file, fmt, vargs);
#if 0
    int len = strlen(fmt);
    if(len > 1 && fmt[len-1]!='\n') {
        fputc('\n', file);
    }
#endif
}

void rpl_debug::logv(const char *fmt, va_list vargs)
{
    if(file == NULL) return;
    if(want_time_log) {
        struct timeval tv1;
        gettimeofday(&tv1, NULL);

        struct tm tm1;
        localtime_r(&tv1.tv_sec, &tm1);

        char tbuf[64];
        strftime(tbuf, sizeof(tbuf), "%F-%T", &tm1);
        fprintf(file, "[%s.%u] ", tbuf, tv1.tv_usec/1000);
    }
    logv_more(fmt, vargs);
}

void rpl_debug::log(const char *fmt, ...)
{
    va_list vargs;
    va_start(vargs,fmt);

    if(flag) {
        logv(fmt, vargs);
    }
}

void rpl_debug::info(const char *fmt, ...)
{
    va_list vargs;
    va_start(vargs,fmt);

    logv(fmt, vargs);
}

void rpl_debug::info_more(const char *fmt, ...)
{
    va_list vargs;
    va_start(vargs,fmt);

    logv_more(fmt, vargs);
}

void rpl_debug::warn(const char *fmt, ...)
{
    va_list vargs;
    va_start(vargs,fmt);

    logv(fmt, vargs);
}

void rpl_debug::error(const char *fmt, ...)
{
    va_list vargs;
    va_start(vargs,fmt);

    logv(fmt, vargs);
}

void rpl_debug::verbose_more(const char *fmt, ...)
{
    va_list vargs;
    va_start(vargs,fmt);

    if(flag) {
        logv_more(fmt, vargs);
    }
}

void rpl_debug::verbose(const char *fmt, ...)
{
    va_list vargs;
    va_start(vargs,fmt);

    if(flag) {
        logv(fmt, vargs);
    }
}

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
