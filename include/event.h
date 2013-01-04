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
        rpl_send_dao = 2,
	rpl_event_max
    };

    rpl_event() { };

    rpl_event(struct timeval &relative, 
              unsigned int sec, unsigned int msec,
              event_types t, const char *reason, rpl_debug *deb) {
        init_event(relative, sec, msec, t, reason, deb);
    };

    rpl_event(unsigned int sec, unsigned int msec,
              event_types t, const char *reason, rpl_debug *deb) {
        struct timeval now;
        gettimeofday(&now, NULL);

        init_event(now, sec, msec, t, reason, deb);
    };

    enum event_types event_type;
    const char *event_name();

    /* invoke this event */
    bool doit(void);
    bool send_dio_all(void);
    bool send_dao_all(void);

    bool passed(struct timeval &now) {
        //fprintf(stderr, "passed([%u,%u] < [%u,%u])\n",
        //        alarm_time.tv_sec, alarm_time.tv_usec,
        //        now.tv_sec, now.tv_usec);

        if(alarm_time.tv_sec  <  now.tv_sec)  return true;
        if(alarm_time.tv_sec  == now.tv_sec && 
           alarm_time.tv_usec <= now.tv_usec) return true;
        return false;
    };
    
    /* calculate against this event */
    struct timeval      occurs_in(struct timeval &now);
    struct timeval      occurs_in(void);     
    const int           miliseconds_util(void);
    const int           miliseconds_util(struct timeval &now);

    void requeue(void);
    void requeue(struct timeval &now);
    void cancel(void);

    /* dump this event for humans */
    void printevent(FILE *out);

    /* the associated iface for this event */
    network_interface  *interface;

    struct timeval      alarm_time;
    const char *getReason() {
        return mReason;
    };

    void set_alarm(struct timeval &relative,
                   unsigned int sec, unsigned int msec)
    {
        last_time  = relative;
        repeat_sec = sec;
        repeat_msec= msec;
        alarm_time.tv_usec = relative.tv_usec + msec*1000;
        alarm_time.tv_sec  = relative.tv_sec  + sec;
        while(alarm_time.tv_usec > 1000000) {
            alarm_time.tv_usec -= 1000000;
            alarm_time.tv_sec++;
        }
    };

    void reset_alarm(unsigned int sec, unsigned int msec) {
        struct timeval now;
        gettimeofday(&now, NULL);
	set_alarm(now, sec, msec);
    };

    dag_network        *mDag;

    /* set to true to remove variable dates from debug output 
     * used by regression testing routines.
     */
    static bool         event_debug_time;          
private:
    void init_event(struct timeval &relative,
                    unsigned int sec, unsigned int msec,
                    event_types t, const char *reason, rpl_debug *deb) {
        set_alarm(relative, sec, msec);
        event_type = t;
        mReason[0]='\0';
        strncat(mReason, reason, sizeof(mReason));
        debug = deb;
	event_number = event_counter++;
    };

    unsigned int        event_number;
    static unsigned int event_counter;
    unsigned int        repeat_sec;
    unsigned int        repeat_msec;
    struct timeval      last_time;
    char mReason[16];
    rpl_debug *debug;
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

typedef std::map<struct timeval,rpl_event *,rpl_eventless>           event_map;
typedef std::map<struct timeval,rpl_event *,rpl_eventless>::iterator event_map_iterator;
typedef std::map<struct timeval,rpl_event *,rpl_eventless>::reverse_iterator event_map_riterator;

void printevents(FILE *out, event_map em);

#endif /* _UNSTRUNG_EVENT_H */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

