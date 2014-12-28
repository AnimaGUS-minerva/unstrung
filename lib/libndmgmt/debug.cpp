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
#include <syslog.h>
#include <time.h>
}

#include "debug.h"

/* XXX: I guess LOG_PERROR could have been used rather than --stderr */
void rpl_debug::open_syslog(void)
{
    if(syslog_open) return;
    openlog(progname, LOG_CONS|LOG_PID, LOG_DAEMON);
    syslog_open = true;
}

void rpl_debug::logv_append(const char *fmt, va_list vargs)
{
    if(logspot == NULL) {
        logspot=syslogbuf;
        sysloglen=sizeof(syslogbuf);
    }
    int len = vsnprintf(logspot, sysloglen, fmt, vargs);
    sysloglen -= len;
    logspot   += len;
}

void rpl_debug::log_append(const char *fmt, ...)
{
    va_list vargs;
    va_start(vargs,fmt);
    logv_append(fmt, vargs);
}

void rpl_debug::logv_syslog_flush(void)
{
    open_syslog();
    syslog(LOG_INFO, "%s", syslogbuf);
}


void rpl_debug::logv_file_flush(void)
{
    if(file == NULL) return;
    fprintf(file, "%s", syslogbuf);
}

void rpl_debug::logv_flush(void)
{
    if(log_file)   logv_file_flush();
    if(log_syslog) logv_syslog_flush();
}

void rpl_debug::logv_more(const char *fmt, va_list vargs)
{
    logv_append(fmt, vargs);
    int len = strlen(fmt);
    if(len > 1 && fmt[len-1]=='\n') {
        needslf=false;
    } else {
        needslf=true;
    }
}
void rpl_debug::logv(const char *fmt, va_list vargs)
{
    if(needslf) {
        /* terminate previous line, flush */
        logv_file_flush();
        logv_syslog_flush();
    }
    if(want_time_log) {
        struct timeval tv1;
        gettimeofday(&tv1, NULL);

        struct tm tm1;
        localtime_r(&tv1.tv_sec, &tm1);

        char tbuf[64];
        strftime(tbuf, sizeof(tbuf), "%F-%T", &tm1);
        log_append("[%s.%u] ", tbuf, tv1.tv_usec/1000);
    }
    logv_append(fmt, vargs);
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

void rpl_debug::debug(unsigned int level, const char *fmt, ...)
{
    va_list vargs;
    va_start(vargs,fmt);

    if(flag_set(level)) {
        logv(fmt, vargs);
    }
}

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
