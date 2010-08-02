#ifndef _UNSTRUNG_EVENT_H
#define _UNSTRUNG_EVENT_H

extern "C" {
#include <sys/time.h>
#include "oswlibs.h"
#include "rpl.h"
}

#include <map>

#include "prefix.h"
#include "iface.h"

class network_interface;
class dag_network;
class rpl_node;
class rpl_eventless;
class rpl_event;

class rpl_event {
public:
    friend class rpl_eventless;

    enum event_types {
        rpl_send_dio = 1,
    };

    rpl_event() { };

    rpl_event(struct timeval &relative,
              unsigned int sec, unsigned int msec,
              event_types t, const char *reason) {
        init_event(relative, sec, msec, t, reason);
    };

    rpl_event(unsigned int sec, unsigned int msec,
              event_types t, const char *reason) {
        struct timeval now;
        gettimeofday(&now, NULL);

        init_event(now, sec, msec, t, reason);
    };

    enum event_types event_type;
    const char *event_name();

    /* invoke this event */
    void doit(void);
    bool passed(struct timeval &now) {
        return (alarm_time.tv_sec  <= now.tv_sec &&
                alarm_time.tv_usec <= now.tv_usec);
    };
    
    /* calculate against this event */
    struct timeval      occurs_in(struct timeval &now);
    struct timeval      occurs_in(void);     
    const int           miliseconds_util(void);
    const int           miliseconds_util(struct timeval &now);

    /* dump this event for humans */
    void printevent(FILE *out);

    struct timeval      alarm_time;

private:
    void init_event(struct timeval &relative,
              unsigned int sec, unsigned int msec,
              event_types t, const char *reason) {
        event_type = t;
        alarm_time.tv_sec  = relative.tv_sec  + sec;
        alarm_time.tv_usec = relative.tv_usec + msec*1000;
        mReason[0]='\0';
        strncat(mReason, reason, sizeof(mReason));
    };

    struct timeval      last_time;
    network_interface  *interface;
    char mReason[16];

};

class rpl_eventless {
public:
    bool operator()(const struct timeval &a, const struct timeval &b) const {
        int match = b.tv_sec - a.tv_sec;
        //printf("compare1 a:%u b:%u match:%d\n", a.tv_sec, b.tv_sec, match);
        if(match < 0) return true;
        if(match > 0) return false;

        match = b.tv_usec - a.tv_usec;
        //printf("compare2 a:%u b:%u match:%d\n", a.tv_usec, b.tv_usec, match);
        if(match < 0) return false;
        return true;
    }
};  

typedef std::map<struct timeval,rpl_event,rpl_eventless>           event_map;
typedef std::map<struct timeval,rpl_event,rpl_eventless>::iterator event_map_iterator;
typedef std::map<struct timeval,rpl_event,rpl_eventless>::reverse_iterator event_map_riterator;

void printevents(FILE *out, event_map em);

#endif /* _UNSTRUNG_EVENT_H */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

