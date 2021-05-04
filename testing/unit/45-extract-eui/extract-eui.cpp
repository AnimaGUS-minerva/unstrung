/*
 * Unit tests for taking a certificate with an rfc822name or otherName, containing an
 * RFC8994 format ULA, and extracting it.
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

#include "devid.h"

/* need this to initialize rpl_nodes */

rpl_debug *deb;
/* TEST1:
 *   an EUI shall be parsable with -
 */
static void t1(void)
{
  /*                              00112233445566778899AABBCCDDEEFF */
  const char *inputACP = "rfc8994+fd739fc23c3440112233445500000000+@acp.example.com";
  ip_subnet sn;
  memset(&sn, 0, sizeof(sn));

  bool success = device_identity::parse_rfc8994string(inputACP, strlen(inputACP), &sn);

  assert(success);
  assert(sn.maskbits == 128);
  assert(sn.addr.u.v6.sin6_addr.s6_addr[0]  == 0xfd);
  assert(sn.addr.u.v6.sin6_addr.s6_addr[1]  == 0x73);
  assert(sn.addr.u.v6.sin6_addr.s6_addr[2]  == 0x9f);
  assert(sn.addr.u.v6.sin6_addr.s6_addr[3]  == 0xc2);
  assert(sn.addr.u.v6.sin6_addr.s6_addr[4]  == 0x3c);
  assert(sn.addr.u.v6.sin6_addr.s6_addr[5]  == 0x34);
  assert(sn.addr.u.v6.sin6_addr.s6_addr[6]  == 0x40);
  assert(sn.addr.u.v6.sin6_addr.s6_addr[7]  == 0x11);
  assert(sn.addr.u.v6.sin6_addr.s6_addr[8]  == 0x22);
  assert(sn.addr.u.v6.sin6_addr.s6_addr[9]  == 0x33);
  assert(sn.addr.u.v6.sin6_addr.s6_addr[10] == 0x44);
  assert(sn.addr.u.v6.sin6_addr.s6_addr[11] == 0x55);
  assert(sn.addr.u.v6.sin6_addr.s6_addr[12] == 0x00);
  assert(sn.addr.u.v6.sin6_addr.s6_addr[13] == 0x00);
  assert(sn.addr.u.v6.sin6_addr.s6_addr[14] == 0x00);
  assert(sn.addr.u.v6.sin6_addr.s6_addr[15] == 0x00);
}

static void t1B(void)
{
  /*                              00112233445566778899AABBCCDDEEFF */
  const char *inputACP = "rfcSELF+fd739fc23c3440112233445500000000+@acp.example.com";
  ip_subnet sn;
  memset(&sn, 0, sizeof(sn));

  bool success = device_identity::parse_rfc8994string(inputACP, strlen(inputACP), &sn);

  assert(success);
  assert(sn.maskbits == 128);
}

static void t2(void)
{
  /*                              00112233445566778899AABBCCDDEEFF */
  const char *inputACP = "rfc8995+fd739fc23c3440112233445500000000+@acp.example.com";
  ip_subnet sn;
  memset(&sn, 0, sizeof(sn));

  bool success = device_identity::parse_rfc8994string(inputACP, strlen(inputACP), &sn);
  assert(!success);
}

static void t3(void)
{
  /*                              00112233445566778899AABBCCDDEEFF */
  const char *inputACP = "rfc8995+fd739fc23c344011ZQ33445500000000+@acp.example.com";
  ip_subnet sn;
  memset(&sn, 0, sizeof(sn));

  bool success = device_identity::parse_rfc8994string(inputACP, strlen(inputACP), &sn);
  assert(!success);
}

static void t4(void)
{
  /*                              00112233445566778899AABBCCDDEEFF */
  const char *inputCert= "f202.crt";

  ip_subnet sn;
  memset(&sn, 0, sizeof(sn));

  device_identity di;

  int loaded = di.load_identity_from_cert(NULL, inputCert);
  assert(loaded == 0);

  bool success = di.parse_rfc8994cert(&sn);
  assert(success);
}

int main(int argc, char *argv[])
{
  deb = new rpl_debug(false, stdout);

  t1();
  t1B();
  t2();
  t3();
  t4();
  exit(0);
}


