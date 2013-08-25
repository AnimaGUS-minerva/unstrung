#ifndef _UNSTRUNG_DEBUG_H
#define _UNSTRUNG_DEBUG_H

#define VERBOSE(X) ((X)->debug && (X)->debug->verbose_test())

extern "C" {
#include <stdarg.h>
}

class rpl_debug {
public:
        rpl_debug(bool verbose, FILE *out) {
                flag = verbose;
                file = out;
                want_time_log = false;
        };

        /* debugging */
	bool                    flag;
	FILE                   *file;
        bool                    want_time_log;
        bool                    verbose_test() {
                return(flag && file!=NULL);
        };

        void set_verbose(FILE *f) {
                file = f;
                flag = true;
        }
#define verbose_file debug->file

        void log(const char *fmt, ...);
        void info(const char *fmt, ...);
        void info_more(const char *fmt, ...);
        void warn(const char *fmt, ...);
        void error(const char *fmt, ...);
        void verbose(const char *fmt, ...);
        void verbose_more(const char *fmt, ...);
        void verbose2(const char *fmt, ...) { /* nothing */};
        void logv(const char *fmt, va_list vargs);
        void logv_more(const char *fmt, va_list vargs);
};

#endif /* _UNSTRUNG_DEBUG_H */

