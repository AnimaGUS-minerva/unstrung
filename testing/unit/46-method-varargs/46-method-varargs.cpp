/*
 * Copyright (C) 2021 Michael Richardson <mcr@sandelman.ca>
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
#include <sys/types.h>
#include <unistd.h>

#include <stdarg.h>
#include <syslog.h>
}

class test_debug {
public:
    test_debug(bool verbose, bool time_log) {
        flag = verbose;
        want_time_log = time_log;
    };

    /* debugging */
    bool                    flag;
    bool                    want_time_log;

    void logv(int level, const char *fmt, ...);
};

void test_debug::logv(int level, const char *fmt, ...)
{
    va_list vargs;
    va_start(vargs, fmt);

    if(want_time_log) {
        struct timeval tv1;
        gettimeofday(&tv1, NULL);

        struct tm tm1;
        localtime_r(&tv1.tv_sec, &tm1);

        char tbuf[64];
        strftime(tbuf, sizeof(tbuf), "%F-%T", &tm1);
        fprintf(stdout, "[%s.%u] ", tbuf, tv1.tv_usec/1000);
    }
    if(flag) {
        vfprintf(stdout, fmt, vargs);
    }
}

int main(int args, char *argv[])
{
    class test_debug *td = new test_debug(true, false);

    td->logv(2, "my %s valentine\n", "funny");
}

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
