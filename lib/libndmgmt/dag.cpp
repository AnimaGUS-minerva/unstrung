#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <errno.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <linux/if.h>           /* for IFNAMSIZ */
#include "oswlibs.h"
#include "rpl.h"

}

#include "iface.h"
#include "dag.h"

class dag_network *dag_network::all_dag = NULL;

dag_network::dag_network(dagid_t n_dagid)
{
        memcpy(mDagid, n_dagid, DAGID_LEN);
}

class dag_network *dag_network::find_by_dagid(dagid_t n_dagid)
{
        class dag_network *dn = dag_network::all_dag;

        while(dn != NULL &&
              dn->cmp_dag(n_dagid)!=0) {
                dn = dn->next;
        }
        
        if(dn==NULL) {
                dn = new dag_network(n_dagid);
        }
        return dn;
}

