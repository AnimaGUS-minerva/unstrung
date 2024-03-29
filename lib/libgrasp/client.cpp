/*
 * Copyright (C) 2016 Michael Richardson <mcr@sandelman.ca>
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

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>

#include "grasp.h"
#include "iface.h"
#include "debug.h"
#include "cbor.h"

#undef GRASP_CLIENT_DEBUG
#ifdef GRASP_CLIENT_DEBUG
#include "hexdump.c"
#endif

/* for generating random numbers, see:
   https://tls.mbed.org/kb/how-to/add-a-random-generator
*/

#include "mbedtls/ctr_drbg.h"

bool grasp_client::open_connection(const char *serverip,
                                   unsigned int port)
{
    if(this->infd != this->outfd && this->outfd != -1) close(this->outfd);
    if(this->infd != -1) close(this->infd);
    this->infd = -1;
    this->outfd = -1;

    struct addrinfo hints;
    struct addrinfo *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family  = AF_UNSPEC;
    hints.ai_socktype= SOCK_STREAM;
    hints.ai_protocol= 0;

    int ret = getaddrinfo(serverip, NULL, &hints, &res);
    if(ret != 0) {
        deb->info("can not lookup %s error: %s\n",
                  serverip, gai_strerror(ret));
        return false;
    }

    int lasterr = 0;

    struct addrinfo *rp;
    int sfd;
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        switch(rp->ai_family) {
        case AF_INET:
            ((struct sockaddr_in *)rp->ai_addr)->sin_port = htons(port);
            break;

        case AF_INET6:
            ((struct sockaddr_in6 *)rp->ai_addr)->sin6_port = htons(port);
            break;
        }

        sfd = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            break;                  /* Success */
        }
        lasterr = errno;

        close(sfd);
    }

    if (rp == NULL) {               /* No address succeeded */
        deb->error("Could not connect to %s:%d: %s\n",
                   serverip, port,
                   strerror(lasterr));
        return false;
    }

    freeaddrinfo(res);           /* No longer needed */
    this->infd  = sfd;
    this->outfd = sfd;
    return true;
}


bool grasp_client::send_cbor(cbor_item_t *cb)
{
    unsigned char * buffer;
    size_t buffer_size,
        length = cbor_serialize_alloc(cb, &buffer, &buffer_size);

    if(write(outfd, buffer, length) != length) {
        deb->error("can not write %u bytes: %s",
                   length, strerror(errno));
        return false;
    }
    queries_outstanding++;

#ifdef GRASP_CLIENT_DEBUG
    /* Pretty-print the result */
    cbor_describe(cb, stdout);
    hexdump(buffer, 0, length);
#endif
    free(buffer);
    return true;
}

cbor_item_t *grasp_client::read_cbor(void)
{
    unsigned char buf[256];
    struct cbor_load_result res;
    unsigned int cnt = read(infd, buf, 256);

    if(cnt == 0) {
        return NULL;
    }

#ifdef GRASP_CLIENT_DEBUG
    hexdump(buf, 0, cnt);
#endif

    cbor_item_t *reply = cbor_load(buf, cnt, &res);

    /* XXX checking res */
    if(!reply) {
        deb->error("can not load reply: decode details");
        return NULL;
    }

#ifdef GRASP_CLIENT_DEBUG
    /* Pretty-print the result */
    cbor_describe(reply, stdout);
#endif

    return reply;
}


grasp_session_id grasp_client::start_query_for_aro(unsigned char eui64[8])
{
    grasp_session_id gsi = generate_random_sessionid(true);
    char eui64buf[EUI64_BUF_LEN];

    deb->info("starting query Registrar for EUI: %s\n",
                rpl_node::fmt_eui64(eui64, eui64buf, sizeof(eui64buf)));

    /* we are writing a GRASP: 3.7.5.  Request Messages */
    /*  request-negotiation-message = [M_REQ_NEG, session-id, objective]*/

    /* Preallocate the new array structure */
    cbor_item_t * root = cbor_new_definite_array(3);
    {
        cbor_item_t * req_neg = cbor_build_uint8(M_REQ_NEG);
        cbor_array_set(root, 0, cbor_move(req_neg));
    }

    {
        cbor_item_t * req_seqno = cbor_build_uint32(gsi);
        cbor_array_set(root, 1, cbor_move(req_seqno));
    }

    /*
     *
     * objective = [objective-name, objective-flags, loop-count, ?any]
     * objective-name = text ;see specification for uniqueness rules
     * loop-count = 0..255
     */
    {
        cbor_item_t * objective = cbor_new_definite_array(4);
        {
            /* Sandelman Software Works PEN
             * http://pen.iana.org/pen/app?page=AssignPenComplete&service=external&sp=S70753
             * with objective name: "6JOIN"
             */
            cbor_item_t * objname   = cbor_build_string("46930:6JOIN");
            cbor_array_set(objective, 0, cbor_move(objname));
        }
        {
            cbor_item_t * objflag   = cbor_build_uint8(1 << F_NEG);
            cbor_array_set(objective, 1, cbor_move(objflag));
        }
        {
            cbor_item_t * loopcount = cbor_build_uint8(2);
            cbor_array_set(objective, 2, cbor_move(loopcount));
        }

        {
            cbor_item_t * aro_box   = cbor_new_definite_array(2);
            cbor_item_t * aro       = cbor_build_bytestring(eui64, 8);
            cbor_array_set(aro_box,   0, cbor_move(aro));
            cbor_array_set(objective, 3, cbor_move(aro_box));

            cbor_array_set(aro_box,   1, cbor_move(cbor_build_string("6p-ipip")));
        }

        cbor_array_set(root, 2, cbor_move(objective));
    }

    send_cbor(root);
    cbor_decref(&root);

    return gsi;
}


bool grasp_client::decode_grasp_reply(cbor_item_t *reply)
{
    if(cbor_typeof(reply) != CBOR_TYPE_ARRAY) {
        deb->info("reply is not array: %d\n", cbor_typeof(reply));
        return false;
    }

    cbor_item_t *msgitem = cbor_array_get(reply, 0);
    unsigned int msgtype = cbor_get_int(msgitem);

    if(msgtype != M_END) {
        deb->info("reply is not M_END\n");
        return false;
    }

    cbor_item_t *session = cbor_array_get(reply, 1);
    grasp_session_id sessionid = cbor_get_int(session);

    cbor_item_t *result = cbor_array_get(reply, 2);
    if(!result) {
        deb->info("result[2] is empty\n");
        return false;
    }

    if(cbor_typeof(result) != CBOR_TYPE_ARRAY) {
        deb->info("result[2] is not array\n");
        return false;
    }

    cbor_item_t *option = cbor_array_get(result, 0);
    if(option) {
        unsigned int optionnum = cbor_get_int(option);
        iface->process_grasp_reply(sessionid, (optionnum == O_ACCEPT));
        return true;
    } else {
        deb->info("option is not present\n");
        return false;
    }
}


bool grasp_client::poll_setup(struct pollfd *fd1)
{
    if(queries_outstanding > 0) {
        fd1->fd = this->infd;
        fd1->events = POLLIN;
        return true;
    } else {
        return false;
    }
}

bool grasp_client::process_grasp_reply(time_t now)
{
    cbor_item_t *reply = read_cbor();
    if(!reply) {
        return false;
    }
    --queries_outstanding;

    return decode_grasp_reply(reply);
}



grasp_session_id grasp_client::generate_random_sessionid(bool init)
{
    grasp_session_id newrand;
#if 0
    int ret = mbedtls_ctr_drbg_random(&ctr_drbg, (unsigned char *)&newrand, sizeof(newrand));
    if( ret != 0 )
    {
        deb->error("can not generate sessionid");
        return 0;
    }

    if(init) {
        /* force upper bit to zero */
        newrand &= 0x7fffffff;
    } else {
        /* force upper bit to one  */
        newrand |= 0x80000000;
    }
#else
    static int lastsession = 123456;
    newrand = ++lastsession;
#endif
    return newrand;
}

void grasp_client::init_query_client(network_interface *iface)
{
    if(iface) {
        iface->join_query_client = this;
    }
}

void grasp_client::init_random(void)
{
    if(!entropy_init) {
        entropy_init = true;

#if defined(HAVE_MBEDTLS)
        /* set up the entropy source */
        mbedtls_entropy_init( &entropy );

        //const char *personalization = "unstrung grasp client";
        mbedtls_ctr_drbg_init( &ctr_drbg );
#endif
#if defined(HAVE_BOOST_RNG)
        // assume that it self seeds?
#endif
    }
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * compile-command: "make programs"
 * End:
 */
