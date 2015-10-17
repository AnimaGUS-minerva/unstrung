#ifndef _UNSTRUNG_EVENT_H
#define _UNSTRUNG_EVENT_H

extern "C" {
#include <sys/time.h>
#include "oswlibs.h"
#include "rpl.h"
}

#include <map>
#include <algorithm>
#include <queue>
#include <vector>
#include <boost/heap/binomial_heap.hpp>

#include "prefix.h"

class network_interface;
class dag_network;
class rpl_node;
class rpl_event;

#define IMMEDIATELY 1  /* ms */

class rpl_eventless {
 public:
    bool operator()(const class rpl_event *a, const class rpl_event *b) const;
};


typedef boost::heap::binomial_heap<class rpl_event *, boost::heap::compare<rpl_eventless> > rpl_event_queue_t;
typedef rpl_event_queue_t::handle_type rpl_event_queue_handle_t;

class rpl_event {
public:
    friend class rpl_eventless;

    enum event_types {
        rpl_send_dio = 1,
        rpl_send_dao = 2,
	rpl_event_max
    };

    rpl_event() {
        deleted = false;
        mDag = NULL;
        inQueue = false;
    };

    rpl_event(struct timeval &relative,
              unsigned int sec, unsigned int msec,
              event_types t, const char *reason, rpl_debug *deb) {
        init_event(relative, sec, msec, t, reason, deb);
    };

    bool deleted;
    ~rpl_event();

    static bool                    faked_time;
    static struct timeval          fake_time;
    static void set_fake_time(struct timeval n) {
	faked_time = true;
	fake_time  = n;
    };
    static void advance_fake_time(void) {
	fake_time.tv_sec++;
    };

    rpl_event(unsigned int sec, unsigned int msec,
              event_types t, const char *reason, rpl_debug *deb) {
        struct timeval now;
	gettimeofday(&now, NULL);
        init_event(now, sec, msec, t, reason, deb);
    };

    enum event_types event_type;
    const char *event_name() const;

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

    void requeue(class rpl_event_queue &list);
    void requeue(class rpl_event_queue &list, struct timeval &now);

    void printevent(FILE *out);

    /* the associated iface for this event */
    network_interface  *interface;

    struct timeval      alarm_time;
    const char *getReason() const {
        return mReason;
    };

    void set_alarm(struct timeval &relative,
                   unsigned int sec, unsigned int msec)
    {
        struct timeval rel = relative;
	if(faked_time) {
	    rel = fake_time;
	}

        last_time  = rel;
        alarm_time.tv_usec = rel.tv_usec + msec*1000;
        alarm_time.tv_sec  = rel.tv_sec  + sec;
        while(alarm_time.tv_usec > 1000000) {
            alarm_time.tv_usec -= 1000000;
            alarm_time.tv_sec++;
        }

	//fprintf(stderr, "%u: alarm for %u/%u + %u/%u\n", event_number, rel.tv_sec, rel.tv_usec, sec, msec);
    };

    void reset_alarm(struct timeval &relative,
                     unsigned int sec, unsigned int msec) {
	set_alarm(relative, sec, msec);

        /* also remember the repeat values on reset */
        repeat_sec = sec;
        repeat_msec= msec;

    };

    dag_network        *mDag;
    bool                inQueue;
    rpl_event_queue_handle_t handle;

    /* set to true to remove variable dates from debug output
     * used by regression testing routines.
     */
    void init_event(struct timeval &relative,
                    unsigned int sec, unsigned int msec,
                    event_types t, const char *reason, rpl_debug *deb) {
        interface = NULL;
        event_type = t;
        mReason[0]='\0';
        strncat(mReason, reason, sizeof(mReason)-1);
        debug = deb;
	event_number = event_counter++;
        repeat_sec = sec;
        repeat_msec= msec;
        set_alarm(relative, sec, msec);
        deleted = false;
        mDag = NULL;
        inQueue = false;
    };

private:
    void _requeue(class rpl_event_queue &list);
    unsigned int        event_number;
    static unsigned int event_counter;
    unsigned int        repeat_sec;
    unsigned int        repeat_msec;
    struct timeval      last_time;
    char mReason[32];
    rpl_debug *debug;
};

class rpl_event_queue {
public:
    rpl_event_queue_t queue;

    rpl_event_queue() {
	make_event_heap();
    };

    void make_event_heap() {
	//make_heap(queue.begin(), queue.end(), rpl_eventless);
    };

    class rpl_event *peek_event(void) {
        return queue.top();
    };

    void update(rpl_event_queue_handle_t b) {
        queue.update(b);
    };

    void eat_event(void) {
        if(!queue.empty()) {
            queue.pop();
        }
    }

    class rpl_event *next_event(void) {
        if(queue.empty()) return NULL;
	rpl_event *n = queue.top();
        eat_event();
        n->inQueue = false;
	return n;
    };

    void add_event(class rpl_event *n) {
        assert(!n->inQueue);
        n->handle = queue.push(n);
        n->inQueue=true;
    };

    /* remove all items from queue */
    void clear(void);

    int size(void) {
	return queue.size();
    };

    /* dump this event for humans */
    void printevents(FILE *out, const char *prefix);

};

#endif /* _UNSTRUNG_EVENT_H */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

