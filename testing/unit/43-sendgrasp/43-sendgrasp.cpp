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

        unsigned int seqno = rand();

        /* we are writing a GRASP: 3.7.5.  Request Messages */
        /*  request-negotiation-message = [M_REQ_NEG, session-id, objective]*/

	/* Preallocate the new array structure */
	cbor_item_t * root = cbor_new_definite_array(3);
        {
          cbor_item_t * req_neg = cbor_build_uint8(M_REQ_NEG);
          cbor_array_set(root, 0, cbor_move(req_neg));
        }

        {
          cbor_item_t * req_seqno = cbor_build_uint32(seqno);
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

          unsigned char mac1[8] = { 0x1, 0x2, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
          {
            cbor_item_t * aro_box   = cbor_new_definite_array(2);
            cbor_item_t * aro       = cbor_build_bytestring(mac1, 8);
            cbor_array_set(aro_box,   0, cbor_move(aro));
            cbor_array_set(objective, 3, cbor_move(aro_box));

            cbor_array_set(aro_box,   1, cbor_move(cbor_build_string("6p-ipip")));
          }

          cbor_array_set(root, 2, cbor_move(objective));
        }

	/* Output: `length` bytes of data in the `buffer` */
	unsigned char * buffer;
	size_t buffer_size,
          length = cbor_serialize_alloc(root, &buffer, &buffer_size);

        FILE *out = fopen("../OUTPUTS/43-6join-grasp.dump", "w");
        if(out) {
          fwrite(buffer, 1, length, out);
          fclose(out);
        }

        hexdump(buffer, 0, length);
	free(buffer);

	fflush(stdout);
	cbor_decref(&root);

        FILE *in = fopen("grasp-reply.dump", "r");
        if(!in) {
          fprintf(stderr, "Can not open grasp-reply.dump: %s", strerror(errno));
          exit(10);
        }

        unsigned char buf[256];
        struct cbor_load_result res;
        unsigned int cnt = fread(buf, 1, 256, in);

        cbor_item_t *reply = cbor_load(buf, cnt, &res);
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


