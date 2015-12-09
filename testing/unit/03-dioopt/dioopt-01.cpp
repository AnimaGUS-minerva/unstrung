/*
 * Unit tests for processing a DIO.
 *
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

extern "C" {
#include "pcap.h"
#include "sll.h"
#include "ether.h"
#include <libgen.h>
#include <string.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <assert.h>
#include <time.h>
}

#include "iface.h"
#include "dag.h"
#include "dio.h"

/* need this to initialize rpl_nodes */

class dag_network *dn;
rpl_debug *deb;
/* TEST1:
 *   a DN shall have a sequence number
 */
static void t1(void)
{
  class rpl_node n1("2001:db8::abcd:0001", dn, deb);

        /* test data from senddio-test-04.out */
        u_int8_t diodata[]={
                /* ICMPv6 9b 01 00 00  */
                /* rpl_instance */0x2a,
                /* rpl_version  */0x01,
                /* rpl_dagrank  */0x00, 0x01,
                /* rpl_mopprf   */0x88,
                /* rpl_dtsn     */0x0a,
                /* rpl_flags    */0x00,
                /* rpl_resv1    */0x00,
                /* rpl_dagid: */
                0x74, 0x68, 0x69, 0x73, 0x69, 0x73, 0x6d, 0x79,
                0x6e, 0x69, 0x63, 0x65, 0x64, 0x61, 0x67, 0x31,

                /* RPL_DStPREFIX */0x08,
                /* length:       */0x0d,
                /* prefixlen:(48)*/0x30,
                /* prf/flags     */0x00,
                /* lifetime:     */0x00, 0x00, 0x00, 0x00,
                /* prefix: */ 0x20, 0x01, 0x0d, 0xb8,
                              0x00, 0x01, 0x00, 0x00,
                /* padding */ 0x00, 0x00, 0x00, 0x00
        };
        int dio_data_len=sizeof(diodata);

        rpl_dio dio(n1, dn, (struct nd_rpl_dio *)diodata,
                    dio_data_len);

        assert(dio.search_subopt(RPL_DIO_DESTPREFIX) != NULL);
        assert(dio.search_subopt(RPL_DIO_DESTPREFIX) == NULL);
}

static void t2(void)
{
  class rpl_node n2("2001:db8::abcd:0002", dn, deb);

        /* test data from senddio-test-04.out */
        u_int8_t diodata[]={
                0x2a, 0x01, 0x00, 0x01,
                0x88, 0x0a, 0x00, 0x00, 0x74, 0x68, 0x69, 0x73,
                0x69, 0x73, 0x6d, 0x79, 0x6e, 0x69, 0x63, 0x65,
                0x64, 0x61, 0x67, 0x31,
                RPL_OPT_PAD0,
                RPL_OPT_PADN, 0x02, 0x04, 0x00,
                RPL_DIO_METRICS, 0x0e, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x30, 0x20, 0x01, 0x0d, 0xb8, 0x00, 0x01, 0x00
        };
        int dio_data_len=sizeof(diodata);

        rpl_dio dio(n2, dn, (struct nd_rpl_dio *)diodata,
                    dio_data_len);

        assert(dio.search_subopt(RPL_DIO_DESTPREFIX) == NULL);
        assert(dio.search_subopt(RPL_DIO_METRICS)    != NULL);
        assert(dio.search_subopt(RPL_OPT_PAD0)       != NULL);
        assert(dio.search_subopt(RPL_OPT_PAD0)       == NULL);
        assert(dio.search_subopt(RPL_OPT_PADN)       != NULL);
        assert(dio.search_subopt(RPL_DIO_METRICS)    == NULL);
        assert(dio.search_subopt(RPL_DIO_DESTPREFIX) == NULL);
}


int main(int argc, char *argv[])
{
  deb = new rpl_debug(false, stdout);
  dn  = new dag_network(1, "1234", deb);
        printf("dioopt-01 t1\n");     t1();
        printf("dioopt-01 t2\n");     t2();

	exit(0);
}


