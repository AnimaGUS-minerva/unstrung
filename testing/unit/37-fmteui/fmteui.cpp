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

/* need this to initialize rpl_nodes */

rpl_debug *deb;
/* TEST1:
 *   an EUI shall be parsable with -
 */
static void t1(void)
{
  const char *inputeui64 = "00-0A-02:B3-4-a-1677";
  unsigned char eui64[8];

  unsigned int eui64len = network_interface::parse_eui2bin(eui64, 8, inputeui64);
  assert(eui64len == 8);
  assert(eui64[0]==0x00);
  assert(eui64[1]==0x0A);
  assert(eui64[2]==0x02);
  assert(eui64[3]==0xB3);
  assert(eui64[4]==0x04);
  assert(eui64[5]==0xa);
  assert(eui64[6]==0x16);
  assert(eui64[7]==0x77);
}

static void t2(void)
{
  const char *inputeui64 = "00-0A-a0-4-a-16";
  unsigned char eui64[8];

  unsigned int eui64len = network_interface::parse_eui2bin(eui64, 8, inputeui64);
  assert(eui64len == 6);
  assert(eui64[0]==0x00);
  assert(eui64[1]==0x0A);
  assert(eui64[2]==0xA0);
  assert(eui64[3]==0x04);
  assert(eui64[4]==0xa);
  assert(eui64[5]==0x16);
}

static void t3(void)
{
  const char *inputeui64 = "00-01-02-03-04-05-06-07";
  unsigned char eui64[8];
  char output[64];

  eui64[0]=0x00;
  eui64[1]=0x01;
  eui64[2]=0x02;
  eui64[3]=0x03;
  eui64[4]=0x04;
  eui64[5]=0x05;
  eui64[6]=0x06;
  eui64[7]=0x07;

  network_interface::fmt_eui(output, sizeof(output), eui64, 8);
  assert(output[strlen(inputeui64)]=='\0');
  assert(strcasecmp(inputeui64, output) == 0);

  assert(network_interface::fmt_eui(output, 5, eui64, 8) == false);

}


int main(int argc, char *argv[])
{
  deb = new rpl_debug(false, stdout);

  t1();
  t2();
  t3();
  exit(0);
}


