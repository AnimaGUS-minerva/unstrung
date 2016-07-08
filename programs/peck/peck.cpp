/*
 * Copyright (C) 2009-2016 Michael Richardson <mcr@sandelman.ca>
 */

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <netinet/ip6.h>

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_fprintf    fprintf
#define mbedtls_printf     printf
#endif

#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/x509.h"
#include "mbedtls/oid.h"

}

#include "iface.h"
#include "debug.h"
#include "fakeiface.h"

static void usage(void)
{
    fprintf(stderr, "Usage: peck [--verbose] ifname\n");
    exit(2);
}

extern "C" {
  static int my_verify( void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags )
  {
    char buf[1024];
    ((void) data);  // some hokey way to say "UNUSED"???

    printf( "\nVerify requested for (Depth %d):\n", depth );
    mbedtls_x509_crt_info( buf, sizeof( buf ) - 1, "", crt );
    printf( "%s", buf );

    if ( ( *flags ) == 0 )
      printf( "  This certificate has no flags\n" );
    else
      {
        mbedtls_x509_crt_verify_info( buf, sizeof( buf ), "  ! ", *flags );
        printf( "%s\n", buf );
      }

    return( 0 );
  }
}

bool read_cert_file( const char *ca_file, const char *certfile )
{
    int ret = 0;
    unsigned char buf[1024];
    mbedtls_net_context server_fd;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
    mbedtls_x509_crt clicert;
    mbedtls_pk_context pkey;
    int i;
    uint32_t flags;
    int verify = 0;
    char *p, *q;

    /*
     * Set to sane values
     */
    //mbedtls_net_free( &server_fd );
    mbedtls_ctr_drbg_init( &ctr_drbg );
    //mbedtls_ssl_init( &ssl );
    //mbedtls_ssl_config_init( &conf );
    mbedtls_x509_crt_init( &cacert );
    mbedtls_x509_crt_init( &clicert );
    /* Zeroize structure as CRL parsing is not supported and we have to pass
       it to the verify function */
    mbedtls_pk_init( &pkey );

    /*
     * 1.1. Load the trusted CA
     */
    mbedtls_printf( "  . Loading the CA root certificate ..." );
    fflush( stdout );

    ret = mbedtls_x509_crt_parse_file( &cacert, ca_file );

    if( ret < 0 )
    {
      fprintf(stderr, " failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", -ret );
      return false;
    }

    mbedtls_printf( " ok (%d skipped)\n", ret );

    {
        mbedtls_x509_crt crt;
        mbedtls_x509_crt *cur = &crt;
        mbedtls_x509_crt *last= &crt;
        mbedtls_x509_crt_init( &crt );

        /*
         * 1.1. Load the certificate(s)
         */
        printf( "\n  . Loading the certificate(s) ..." );

        ret = mbedtls_x509_crt_parse_file( &crt, certfile );

        if( ret < 0 )
        {
            printf( " failed\n  !  mbedtls_x509_crt_parse_file returned %d\n\n", ret );
            mbedtls_x509_crt_free( &crt );
            goto exit;
        }

        printf( " ok\n" );

        /*
         * 1.2 Print the certificate(s)
         */
        while( cur != NULL )
        {
          printf( "  . Peer certificate information    ...\n" );
          ret = mbedtls_x509_crt_info( (char *) buf, sizeof( buf ) - 1, "      ",
                                 cur );
          if( ret == -1 )
            {
              printf( " failed\n  !  mbedtls_x509_crt_info returned %d\n\n", ret );
              mbedtls_x509_crt_free( &crt );
              return false;
            }

            printf( "%s\n", buf );

            last = cur;

            cur = cur->next;
        }

        ret = 0;

        /*
         * 1.3 Verify the certificate
         */
        if( true )
        {
          printf( "  . Verifying X.509 certificate..." );

          if( ( ret = mbedtls_x509_crt_verify( &crt, &cacert, NULL, NULL, &flags,
                                         my_verify, NULL ) ) != 0 )
            {
                char vrfy_buf[512];

                printf( " failed\n" );

                mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );

                printf( "%s\n", vrfy_buf );
                goto exit;
            }
            else
                printf( " ok\n" );
        }

        /* now process the last certificate that has the right subjectAltName */
        if(last) {
          const mbedtls_x509_sequence *cur_seq = &last->subject_alt_names;
          unsigned int seqno=0;

          while(cur_seq != NULL) {
            mbedtls_x509_buf extn_oid = {0, 0, NULL};
            unsigned char *end_ext_data = NULL;
            size_t end_ext_len;
            size_t utf8len, oNameLen;
            char eui64buf[64];
            int ret;

            /* get the OID from the next bytes. */
            end_ext_data = cur_seq->buf.p;
            end_ext_len  = cur_seq->buf.len;

            if( ( ret = mbedtls_asn1_get_tag( &end_ext_data,
                                              end_ext_data+cur_seq->buf.len, &extn_oid.len,
                                              MBEDTLS_ASN1_OID ) ) != 0 ) {
              return ret;
            }

            extn_oid.p    = end_ext_data;
            end_ext_data += extn_oid.len;
            end_ext_len  -= extn_oid.len;

            if( extn_oid.len != MBEDTLS_OID_SIZE( MBEDTLS_OID_EUI64 ) ||
                memcmp( extn_oid.p, MBEDTLS_OID_EUI64, extn_oid.len ) != 0 ) {
              continue;
            }

            if( ( ret = mbedtls_asn1_get_tag( &end_ext_data,
                                              end_ext_data+end_ext_len,
                                              &oNameLen,
                                              MBEDTLS_ASN1_PRIVATE ) ) != 0 ) {
              continue;
            }
            end_ext_len  -= 2;

            if( ( ret = mbedtls_asn1_get_tag( &end_ext_data,
                                              end_ext_data+end_ext_len,
                                              &utf8len,
                                              MBEDTLS_ASN1_UTF8_STRING ) ) != 0 ) {
              continue;
            }

            end_ext_len  = utf8len;
            if(end_ext_len > sizeof(eui64buf)-1) {
              end_ext_len = sizeof(eui64buf)-1;
            }
            memcpy(eui64buf, end_ext_data, end_ext_len);

            printf("\n%u: name %s\n", seqno, eui64buf);

            cur_seq = cur_seq->next;
            seqno++;
          }
        }

        mbedtls_x509_crt_free( &crt );
    }

exit:

    //mbedtls_net_free( &server_fd );
    mbedtls_x509_crt_free( &cacert );
    mbedtls_x509_crt_free( &clicert );
    mbedtls_pk_free( &pkey );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

    if( ret < 0 )
      return false;

    return true;
}



int main(int argc, char *argv[])
{
  int verbose = 0;
  bool fakesend=false;
  bool initted =false;
  char c;
  const char *micpemfile = "/boot/mic.pem";
  const char *micprivfile= "/boot/mic.priv";
  const char *manufact_ca= "/boot/manufacturer.pem";
  class rpl_debug *deb;
  struct option longoptions[]={
    {"help",     0, NULL, '?'},
    {"fake",     0, NULL, 'T'},
    {"mic",      1, NULL, 'm'},
    {"privmic",  1, NULL, 'M'},
    {"manuca",   1, NULL, 'R'},
    {"verbose",  0, NULL, 'v'},
    {0,0,0,0},
  };

  deb = new rpl_debug(verbose, stderr);

  while((c=getopt_long(argc, argv, "?hm:vFM:RV", longoptions, NULL))!=EOF){
    switch(c) {
    case 'T':
      if(initted) {
        fprintf(stderr, "--fake MUST be first argument\n");
        exit(16);
      }
      fakesend=true;
      break;

    case 'v':
      verbose++;
      deb->set_verbose(stderr);
      break;

    case 'V':
      fprintf(stderr, "Version: 1.0\n");
      usage();
      break;

    case 'm':
      micpemfile = strdup(optarg);
      break;

    case 'R':
      manufact_ca = strdup(optarg);
      break;

    case 'M':
      micprivfile= strdup(optarg);
      break;

    case '?':
    case 'h':
    default:
      usage();
      break;

    }
    initted = true;
  }

  if(!initted) {
    if(fakesend) {
      pcap_network_interface::scan_devices(deb, false);
    } else {
      network_interface::scan_devices(deb, false);
    }
  }

  /* open the certificate file */
  if(!read_cert_file( manufact_ca, micpemfile)) {
    exit(10);
  }




  for(int ifnum = optind; ifnum < argc; ifnum++) {
    class pcap_network_interface *iface = NULL;
    iface = (pcap_network_interface *)pcap_network_interface::find_by_name(argv[ifnum]);

    printf("working on interface: %s\n", argv[ifnum]);
  }


  exit(0);
}

