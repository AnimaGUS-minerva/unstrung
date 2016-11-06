#ifndef _UNSTRUNG_DEBUG_H
#define _UNSTRUNG_DEBUG_H

#define VERBOSE(X) ((X)->debug && (X)->debug->verbose_test())

extern "C" {
#include <stdarg.h>
#include <syslog.h>
}

enum debug_level {
  RPL_DEBUG_NETINPUT = 1 << 1,
  RPL_DEBUG_NETLINK  = 1 << 2,
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
        void close_log(void) {
          logv_flush(LOG_ERR);
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
          const char *base = strrchr(a_progname, '/');
          if(base == NULL) base = a_progname;
          this->progname[0]='\0';
          strncat(this->progname, basename(a_progname), sizeof(this->progname));
          syslog_open = false;  /* so that openlog will be called again */
        };

        void set_verbose(FILE *f) {
                file = f;
                flag = true;
        };
        void set_debug_flags(int flags) {
          debug_flags = flags;
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
        void verbose2(const char *fmt, ...) {
#if 1
          va_list vargs;
          va_start(vargs, fmt);
          logv(LOG_DEBUG, fmt, vargs);
#endif
        };
        void debug(unsigned int level, const char *fmt, ...);
        void logv(int level, const char *fmt, va_list vargs);
        void logv_more(int level, const char *fmt, va_list vargs);
        void flush(void);
 private:
        void open_syslog(void);
        void logv_flush(int level);
        void logv_file_flush(int level);
        void logv_syslog_flush(int level);
        void log_append(int level, const char *fmt, ...);
        void logv_append(int level, const char *fmt, va_list vargs);

        /*
         * annoyingly: basename(3) seems not to be in ulibc, and the boost file path
         * manipulations are undergoing a version change among the boost libraries available
         * on the target machines we care about in 2015.
         */
        const char *basename(const char *filename) {
          const char *p;
          const char *slash;

          slash = filename;
          /* find end of filename, noting where last slash is */
          for(p=filename; *p!='\0'; p++) {
            if(*p == '/') slash = p;
          }
          return slash;
        };

};

#endif /* _UNSTRUNG_DEBUG_H */

