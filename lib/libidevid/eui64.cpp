/*
 * Copyright (C) 2009-2021 Michael Richardson <mcr@sandelman.ca>
 *
 * SEE ../../LICENSE
 *
 */

extern "C" {
#include <stdio.h>
#include <stdlib.h>

#include <netinet/ip6.h>
#include "hexdump.c"
}

#include "mbedtls.h"
#include "devid.h"
#include "dag.h"
#include "iface.h"

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

/* returns 0 on success */
int device_identity::load_identity_from_cert( const char *ca_file, const char *certfile )
{
    int ret = 0;
    unsigned char buf[1024];
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
    mbedtls_x509_crt *cur;
    mbedtls_x509_crt *cert;
    mbedtls_pk_context pkey;
    int i;
    uint32_t flags;
    int verify = 0;
    char *p, *q;

    /*
     * Set to sane values
     */
    cert = (mbedtls_x509_crt *)mbedtls_calloc(1, sizeof(*cert));
    cur  = cert;
    mbedtls_x509_crt_init( cert );
    mbedtls_ctr_drbg_init( &ctr_drbg );
    mbedtls_x509_crt_init( &cacert );

    /* Zeroize structure as CRL parsing is not supported and we have to pass
       it to the verify function */
    mbedtls_pk_init( &pkey );

    if(ca_file) {
        /*
         * 1.1. Load the trusted CA
         */
        mbedtls_printf( "  . Loading the CA root certificate ..." );
        fflush( stdout );

        ret = mbedtls_x509_crt_parse_file( &cacert, ca_file );

        if( ret < 0 )
            {
                fprintf(stderr, " failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", -ret );
                return 1;
            }

        mbedtls_printf( " ok (%d skipped)\n", ret );
    }

    /*
     * 1.1. Load the certificate(s)
     */
    printf( "\n  . Loading the certificate at: %s ...", certfile);

    ret = mbedtls_x509_crt_parse_file( cert, certfile );

    if( ret < 0 )
      {
        char errorbuf[256];
        mbedtls_strerror( ret, errorbuf, sizeof(errorbuf));
        printf( " failed\n  !  mbedtls_x509_crt_parse_file returned %d: %s\n\n", ret, errorbuf );

        mbedtls_x509_crt_free( cert );
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
            goto exit;
          }

        printf( "%s\n", buf );

        cur = cur->next;
      }

    ret = 0;

    if(ca_file) {
        /*
         * 1.3 Verify the certificate
         */
        printf( "  . Verifying X.509 certificate..." );

        if( ( ret = mbedtls_x509_crt_verify( cert, &cacert, NULL, NULL, &flags,
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

exit:
    mbedtls_x509_crt_free( &cacert );
    mbedtls_pk_free( &pkey );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

    if( ret < 0 ) {
      mbedtls_x509_crt_free( cert );
      mbedtls_free(cert);
      return 2;
    }

    this->cert = cert;
    return 0;
}

/* true/false about whether it could convert */
bool device_identity::parse_rfc8994cert(ip_subnet *sn)
{
    /* now process the certificate to find the serialNumber from the
     * subjectName
     */
    mbedtls_x509_name *subject = &cert->subject;

    mbedtls_x509_name *sn_attr =
        mbedtls_asn1_find_named_data( subject,
                                      MBEDTLS_OID_PKCS9_EMAIL,
                                      MBEDTLS_OID_SIZE(MBEDTLS_OID_PKCS9_EMAIL));

    if(sn_attr == NULL) {
        return false;
    }


    return parse_rfc8994string((const char *)sn_attr->val.p, sn_attr->val.len, sn);
}

int device_identity::extract_eui64_from_cert(unsigned char *eui64,
                                             char *eui64buf, unsigned int eui64buf_len)
{
  /* now process the last certificate that has the right subjectAltName */
  const mbedtls_x509_sequence *cur_seq = &cert->subject_alt_names;
  unsigned int seqno=0;

  for(; cur_seq != NULL; cur_seq = cur_seq->next) {
    mbedtls_x509_buf extn_oid = {0, 0, NULL};
    unsigned char *end_ext_data = NULL;
    size_t end_ext_len;
    size_t utf8len, oNameLen;
    int ret;

    seqno++;
    /* get the OID from the next bytes. */
    end_ext_data = cur_seq->buf.p;
    end_ext_len  = cur_seq->buf.len;

    if( ( ret = mbedtls_asn1_get_tag( &end_ext_data,
                                      end_ext_data+cur_seq->buf.len, &extn_oid.len,
                                      MBEDTLS_ASN1_OID ) ) != 0 ) {
      return -1;
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
    if(end_ext_len > eui64buf_len-1) {
      end_ext_len = eui64buf_len-1;
    }
    memcpy(eui64buf, end_ext_data, end_ext_len);
    eui64buf[end_ext_len]='\0';

    unsigned int eui64len = end_ext_len;

    /* see if eui64 if printable or not */
    if(eui64buf[2]=='-' || eui64buf[2]==':') {
      /* convert printable form to binary, and then format it again */

      eui64len = network_interface::parse_eui2bin(eui64, 8, eui64buf);
      if(eui64len != 6 && eui64len != 8) {
        printf("seqno: %d invalid EUI64 found: %s\n", seqno, eui64buf);
        return false;
      }
    } else {
      /* assume it is binary! */
      memcpy(eui64, eui64buf, 8);
    }

    network_interface::fmt_eui(eui64buf, eui64buf_len, eui64, eui64len);

    return eui64len;

  }

  return -1;
}



/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

