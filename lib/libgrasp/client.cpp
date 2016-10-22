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

#include "grasp.h"
#include "debug.h"
#include "cbor.h"
#include "hexdump.c"

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
        deb->error("Could not connect: %s", strerror(lasterr));
        return false;
    }

    freeaddrinfo(res);           /* No longer needed */
    this->infd  = sfd;
    this->outfd = sfd;
}

bool grasp_client::open_fake_connection(const char *outfile, const char *infile)
{
    infd = open(infile, O_RDONLY);
    if(infd == -1) {
        deb->error("can not open %s for input: %s", infile, strerror(errno));
        return false;
    }

    outfd = open(outfile, O_WRONLY|O_TRUNC|O_CREAT, 0640);
    if(outfd == -1) {
        deb->error("can not open %s for output: %s", outfile, strerror(errno));
        return false;
    }
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

    hexdump(buffer, 0, length);
    free(buffer);
    return true;
}

cbor_item_t *grasp_client::read_cbor(void)
{
    unsigned char buf[256];
    struct cbor_load_result res;
    unsigned int cnt = read(infd, buf, 256);

    cbor_item_t *reply = cbor_load(buf, cnt, &res);

    /* XXX checking res */
    if(!reply) {
        deb->error("can not load reply: decode details");
        return NULL;
    }

    return reply;
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * compile-command: "make programs"
 * End:
 */
