#ifndef _UNSTRUNG_FAKEIFACE_H_
#define _UNSTRUNG_FAKEIFACE_H_

extern "C" {
#include "pcap.h"
#include "sll.h"
#include "ether.h"
#include <libgen.h>
#include <string.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>

}

extern int process_infile(char *infile, char *outfile);



#endif /* _UNSTRUNG_FAKEIFACE_H_ */

