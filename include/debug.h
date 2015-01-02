#ifndef _UNSTRUNG_DEBUG_H
#define _UNSTRUNG_DEBUG_H

#define VERBOSE(X) ((X)->debug && (X)->debug->verbose_test())

extern "C" {
#include <stdarg.h>
}

enum debug_level {
  RPL_DEBUG_NETINPUT = 1 << 1,
};

class rpl_debug {
public:
        rpl_debug(bool verbose, FILE *out) {
                flag = verbose;
                debug_flags = 0;
                file = out;
                want_time_log = false;
                log_file   = true;
                log_syslog = false;
                this->progname[0]='S';
                this->progname[1]='\0';
        };

        /* debugging */
	bool                    flag;
        bool                    log_syslog;
        bool                    log_file;
        bool                    syslog_open;
        char                   *logspot;
        bool                    needslf;
        char                    syslogbuf[1024];
        int                     sysloglen;
	unsigned int            debug_flags;
	FILE                   *file;
        char                    progname[64];
        bool                    want_time_log;
        bool                    verbose_test() {
                return(flag && file!=NULL);
        };

        void set_progname(const char *a_progname) {
          this->progname[0]='\0';
          strncat(this->progname, a_progname, sizeof(this->progname));
          syslog_open = false;  /* so that openlog will be called again */
        };

        void set_verbose(FILE *f) {
                file = f;
                flag = true;
        };
        void set_debug_flag(enum debug_level dl) {
          debug_flags |= dl;
        };
        void clear_debug_flag(enum debug_level dl) {
          debug_flags &= dl ^ 0xfffffff;
        };
        bool flag_set(unsigned int level) { return(level & this->debug_flags); };
#define verbose_file debug->file

        void log(const char *fmt, ...);
        void info(const char *fmt, ...);
        void info_more(const char *fmt, ...);
        void warn(const char *fmt, ...);
        void error(const char *fmt, ...);
        void verbose(const char *fmt, ...);
        void verbose_more(const char *fmt, ...);
        void verbose2(const char *fmt, ...) { /* nothing */};
        void debug(unsigned int level, const char *fmt, ...);
        void logv(const char *fmt, va_list vargs);
        void logv_more(const char *fmt, va_list vargs);
 private:
        void open_syslog(void);
        void logv_flush(void);
        void logv_file_flush(void);
        void logv_syslog_flush(void);
        void log_append(const char *fmt, ...);
        void logv_append(const char *fmt, va_list vargs);

};

#endif /* _UNSTRUNG_DEBUG_H */

