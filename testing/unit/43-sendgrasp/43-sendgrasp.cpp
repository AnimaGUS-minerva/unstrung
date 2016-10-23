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

        cbor_item_t *reply = gc.read_cbor();
        if(!reply) {
          fprintf(stderr, "Can not load reply: decode details");
          exit(12);
        }
        /* Pretty-print the result */
        cbor_describe(reply, stdout);

        if(cbor_typeof(reply) != CBOR_TYPE_ARRAY) {
          fprintf(stderr, "GRASP objects should be array\n");
          exit(13);
        }

        cbor_item_t *msgitem = cbor_array_get(reply, 0);
        unsigned int msgtype = cbor_get_int(msgitem);

        if(msgtype != M_END) {
          fprintf(stderr, "GRASP response should be M_END\n");
          exit(13);
        }

        cbor_item_t *session = cbor_array_get(reply, 1);
        unsigned int sessionid = cbor_get_int(session);
        printf("session-id: %04x\n", sessionid);

        cbor_item_t *result = cbor_array_get(reply, 2);
        if(!result) {
          fprintf(stderr, "GRASP response does not contain result option\n");
          exit(14);
        }

        cbor_item_t *option = cbor_array_get(result, 0);
        unsigned int optionnum = cbor_get_int(option);
        printf("option_num: %d\n", optionnum);

        if(optionnum == O_ACCEPT) {
          printf("accept eui64\n");
        } else {
          printf("decline eui64\n");
        }

        deb->close_log();
	exit(0);
}


