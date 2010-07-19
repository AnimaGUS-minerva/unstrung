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

class rpl_event {
public:
    friend class rpl_eventless;

    rpl_event() { };

    rpl_event(unsigned int sec, unsigned int msec, const char *reason) {
        interval.tv_sec  = sec;
        interval.tv_usec = msec*1000;
        mReason[0]='\0';
        strncat(mReason, reason, sizeof(mReason));
    };

    enum {
        rpl_send_dio = 1,
    } event_type;

    struct timeval      interval;
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


#endif /* _UNSTRUNG_EVENT_H */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

