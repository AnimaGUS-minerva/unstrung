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
        };

        /* debugging */
	bool                    flag;
	FILE                   *file;
        bool                    verbose_test() {
                return(flag && file!=NULL);
        };
#define verbose_file debug->file

        void log(const char *fmt, ...);
        void logv(const char *fmt, va_list vargs);
};

#endif /* _UNSTRUNG_DEBUG_H */

