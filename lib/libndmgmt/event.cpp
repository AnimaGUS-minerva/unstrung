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

/*
 * this file is just for setup and maintenance of interface,
 * it does not really do any heavy lifting.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <errno.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <poll.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <linux/if.h>           /* for IFNAMSIZ */
#include <time.h>
#include "rpl.h"

#ifndef IPV6_ADDR_LINKLOCAL
#define IPV6_ADDR_LINKLOCAL   0x0020U
#endif
}

#include "iface.h"
#include "event.h"

bool                    rpl_event::faked_time;
struct timeval          rpl_event::fake_time;
unsigned int            rpl_event::event_counter = 1;

rpl_event::~rpl_event(void)
{
    if(mDag) {
        mDag->clear_event(this);
    }
    deleted = true;
}

bool rpl_eventless::operator()(const class rpl_event *a, const class rpl_event *b) const
{
    int match = b->alarm_time.tv_sec - a->alarm_time.tv_sec;
#if 0
    printf("  comp a=%s b=%s 1 a:%010u < b:%010u match:%d\n",
	   a->getReason(), b->getReason(),
	   a->alarm_time.tv_sec, b->alarm_time.tv_sec, match);
#endif
    if(match > 0) return false;
    if(match < 0) return true;

    match = b->alarm_time.tv_usec - a->alarm_time.tv_usec;
#if 0
    printf("                     2 a:%010u < b:%10u match:%d\n", a->alarm_time.tv_usec, b->alarm_time.tv_usec, match);
#endif
    if(match > 0) return false;
    return true;
};

bool rpl_event::doit(void)
{

    debug->log("invoked doit(%s) on rpl_event (if_name=%s)\n",
	       event_name(),
	       interface ? interface->get_if_name() : "none");
    switch(event_type) {
    case rpl_send_dio:
	network_interface::send_dio_all(mDag);
        return true;

    case rpl_send_dao:
        if(mDag!=NULL) {
            debug->log("event send_dao to parent\n");
            mDag->send_dao();
        }
        return true;

    default:
        debug->log("invalid event %d\n", event_type);
        break;
    }
    return false;
}

void rpl_event::_requeue(class rpl_event_queue &list) {
    if(this->inQueue) {
        list.update(handle);
        //list.printevents(debug->file, "requeue2 ");
    } else {
        list.add_event(this);
    }
}

void rpl_event::requeue(class rpl_event_queue &list) {
    debug->verbose("inserting event #%u at %u/%u %u\n",
		   event_number, alarm_time.tv_sec, alarm_time.tv_usec, inQueue);

    _requeue(list);
}

void rpl_event::requeue(class rpl_event_queue &list, struct timeval &now) {
    debug->verbose("re-inserting event #%u repeat: %u/%u %u\n",
		   event_number, repeat_sec, repeat_msec, inQueue);

    set_alarm(now, repeat_sec, repeat_msec);

    _requeue(list);
}



const char *rpl_event::event_name() const
{
    switch(event_type) {
    case rpl_send_dio:
        return "send_dio";
    case rpl_send_dao:
        return "send_dao";
    }

    return "<unknown-event>";
}

void rpl_event::printevent(FILE *out)
{
    char b1[256];
    struct tm tm1;
    struct timeval tmp_alarm_time = alarm_time;

    gmtime_r(&alarm_time.tv_sec, &tm1);

    strftime(b1, sizeof(b1), "%Y-%B-%d %r", &tm1);
    fprintf(out, "event#%u(%s) at (%s)<%u:%u>, type: %s",
	    event_number,
            mReason,  b1, tmp_alarm_time.tv_sec, tmp_alarm_time.tv_usec,
            event_name());
}

struct timeval rpl_event::occurs_in(struct timeval &now) {
    int usec_interval = alarm_time.tv_usec - now.tv_usec;
    int borrow = 0;
    struct timeval diff;

    if(usec_interval < 0) {
        usec_interval += 1000000;
        borrow = 1;
    }

    diff.tv_usec = usec_interval;
    diff.tv_sec = alarm_time.tv_sec - now.tv_sec - borrow;
    return diff;
}

/*
 * assumes that the event is in the future.
 */
const int rpl_event::miliseconds_util(struct timeval &now) {
    unsigned int sec = alarm_time.tv_sec - now.tv_sec;
    int usec = alarm_time.tv_usec - now.tv_usec;

    return (sec*1000 + usec / 1000);
}

const int rpl_event::miliseconds_util() {
    struct timeval now;
    gettimeofday(&now, NULL);

    return miliseconds_util(now);
}

struct timeval rpl_event::occurs_in() {
    struct timeval now;

    gettimeofday(&now, NULL);
    return occurs_in(now);
}

/* dump this event for humans */
void rpl_event_queue::printevents(FILE *out, const char *prefix) {
    int i = 0;
    rpl_event_queue_t::iterator one = queue.begin();

    fprintf(out, "event list (%u events)\n", queue.size());
    while(one != queue.end()) {
	fprintf(out, "%s%d: ", prefix, i);
	(*one)->printevent(out);
	fprintf(out, "\n");
	i++; one++;
    }
};

/* remove all items from queue */
void rpl_event_queue::clear(void) {
    rpl_event_queue_t::iterator one = queue.begin();

    while(one != queue.end()) {
        (*one)->inQueue = false;
        one++;
    }
    queue.clear();
};


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
