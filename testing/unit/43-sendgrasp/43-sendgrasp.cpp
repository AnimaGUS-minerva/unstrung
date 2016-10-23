#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

#include "fakeiface.h"

extern "C" {
#include "hexdump.c"
#include "grasp.h"
#include "pcap.h"
#include "sll.h"
#include "ether.h"
#include <libgen.h>
#include <string.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <net/if_arp.h>
#include <sys/types.h>
#include <cbor.h>
#include <poll.h>

}

int main(int argc, char *argv[])
{
        rpl_debug *deb = new rpl_debug(true, stdout);
        deb->want_time_log = false;

        grasp_client gc(deb);
        gc.init_regress_random();

        for(int i=0; i<16; i++) {
          fprintf(stderr, "random number check seqno[%u]: %08x\n", i, gc.generate_random_sessionid(true));
        }

        gc.open_fake_connection("../OUTPUTS/43-6join-grasp.dump",
                                "grasp-reply.dump");

        unsigned char mac1[8] = { 0x1, 0x2, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
        unsigned int seqno = gc.start_query_for_aro(mac1);

        struct pollfd fds[2];
        fds[0].fd = gc.server_fd();
        fds[0].events = POLLIN;
        int nfds = 1;

        int n = poll(fds, nfds, 1);
        if(n >= 1) {
          cbor_item_t *reply = gc.read_cbor();

          /* Pretty-print the result */
          cbor_describe(reply, stdout);

          if(cbor_typeof(reply) != CBOR_TYPE_ARRAY) {
            fprintf(stderr, "GRASP objects should be array\n");
            exit(13);
          }

          if(!gc.decode_grasp_reply(reply)) {
            fprintf(stderr, "Can not load reply: decode details");
            exit(12);
          }
        }

        deb->close_log();
	exit(0);
}


