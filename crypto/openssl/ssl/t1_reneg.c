
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <openssl/objects.h>
#include "ssl_locl.h"

/* Add the client's renegotiation binding */
int ssl_add_clienthello_renegotiate_ext(SSL *s, unsigned char *p, int *len,
					int maxlen)
    {
    if(p)
        {
	if((s->s3->previous_client_finished_len+1) > maxlen)
            {
            SSLerr(SSL_F_SSL_ADD_CLIENTHELLO_RENEGOTIATE_EXT,SSL_R_RENEGOTIATE_EXT_TOO_LONG);
            return 0;
            }
            
        /* Length byte */
	*p = s->s3->previous_client_finished_len;
        p++;

        memcpy(p, s->s3->previous_client_finished,
	       s->s3->previous_client_finished_len);
#ifdef OPENSSL_RI_DEBUG
    fprintf(stderr, "%s RI extension sent by client\n",
		s->s3->previous_client_finished_len ? "Non-empty" : "Empty");
#endif
        }
    
    *len=s->s3->previous_client_finished_len + 1;

 
    return 1;
    }

/* Parse the client's renegotiation binding and abort if it's not
   right */
int ssl_parse_clienthello_renegotiate_ext(SSL *s, unsigned char *d, int len,
					  int *al)
    {
    int ilen;

    /* Parse the length byte */
    if(len < 1)
        {
        SSLerr(SSL_F_SSL_PARSE_CLIENTHELLO_RENEGOTIATE_EXT,SSL_R_RENEGOTIATION_ENCODING_ERR);
        *al=SSL_AD_ILLEGAL_PARAMETER;
        return 0;
        }
    ilen = *d;
    d++;

    /* Consistency check */
    if((ilen+1) != len)
        {
        SSLerr(SSL_F_SSL_PARSE_CLIENTHELLO_RENEGOTIATE_EXT,SSL_R_RENEGOTIATION_ENCODING_ERR);
        *al=SSL_AD_ILLEGAL_PARAMETER;
        return 0;
        }

    /* Check that the extension matches */
    if(ilen != s->s3->previous_client_finished_len)
        {
        SSLerr(SSL_F_SSL_PARSE_CLIENTHELLO_RENEGOTIATE_EXT,SSL_R_RENEGOTIATION_MISMATCH);
        *al=SSL_AD_HANDSHAKE_FAILURE;
        return 0;
        }
    
    if(memcmp(d, s->s3->previous_client_finished,
	      s->s3->previous_client_finished_len))
        {
        SSLerr(SSL_F_SSL_PARSE_CLIENTHELLO_RENEGOTIATE_EXT,SSL_R_RENEGOTIATION_MISMATCH);
        *al=SSL_AD_HANDSHAKE_FAILURE;
        return 0;
        }
#ifdef OPENSSL_RI_DEBUG
    fprintf(stderr, "%s RI extension received by server\n",
				ilen ? "Non-empty" : "Empty");
#endif

    s->s3->send_connection_binding=1;

    return 1;
    }

/* Add the server's renegotiation binding */
int ssl_add_serverhello_renegotiate_ext(SSL *s, unsigned char *p, int *len,
					int maxlen)
    {
    if(p)
        {
        if((s->s3->previous_client_finished_len +
            s->s3->previous_server_finished_len + 1) > maxlen)
            {
            SSLerr(SSL_F_SSL_ADD_SERVERHELLO_RENEGOTIATE_EXT,SSL_R_RENEGOTIATE_EXT_TOO_LONG);
            return 0;
            }
        
        /* Length byte */
        *p = s->s3->previous_client_finished_len + s->s3->previous_server_finished_len;
        p++;

        memcpy(p, s->s3->previous_client_finished,
	       s->s3->previous_client_finished_len);
        p += s->s3->previous_client_finished_len;

        memcpy(p, s->s3->previous_server_finished,
	       s->s3->previous_server_finished_len);
#ifdef OPENSSL_RI_DEBUG
    fprintf(stderr, "%s RI extension sent by server\n",
    		s->s3->previous_client_finished_len ? "Non-empty" : "Empty");
#endif
        }
    
    *len=s->s3->previous_client_finished_len
	+ s->s3->previous_server_finished_len + 1;
    
    return 1;
    }

/* Parse the server's renegotiation binding and abort if it's not
   right */
int ssl_parse_serverhello_renegotiate_ext(SSL *s, unsigned char *d, int len,
					  int *al)
    {
    int expected_len=s->s3->previous_client_finished_len
	+ s->s3->previous_server_finished_len;
    int ilen;

    /* Check for logic errors */
    OPENSSL_assert(!expected_len || s->s3->previous_client_finished_len);
    OPENSSL_assert(!expected_len || s->s3->previous_server_finished_len);
    
    /* Parse the length byte */
    if(len < 1)
        {
        SSLerr(SSL_F_SSL_PARSE_SERVERHELLO_RENEGOTIATE_EXT,SSL_R_RENEGOTIATION_ENCODING_ERR);
        *al=SSL_AD_ILLEGAL_PARAMETER;
        return 0;
        }
    ilen = *d;
    d++;

    /* Consistency check */
    if(ilen+1 != len)
        {
        SSLerr(SSL_F_SSL_PARSE_SERVERHELLO_RENEGOTIATE_EXT,SSL_R_RENEGOTIATION_ENCODING_ERR);
        *al=SSL_AD_ILLEGAL_PARAMETER;
        return 0;
        }
    
    /* Check that the extension matches */
    if(ilen != expected_len)
        {
        SSLerr(SSL_F_SSL_PARSE_SERVERHELLO_RENEGOTIATE_EXT,SSL_R_RENEGOTIATION_MISMATCH);
        *al=SSL_AD_HANDSHAKE_FAILURE;
        return 0;
        }

    if(memcmp(d, s->s3->previous_client_finished,
	      s->s3->previous_client_finished_len))
        {
        SSLerr(SSL_F_SSL_PARSE_SERVERHELLO_RENEGOTIATE_EXT,SSL_R_RENEGOTIATION_MISMATCH);
        *al=SSL_AD_HANDSHAKE_FAILURE;
        return 0;
        }
    d += s->s3->previous_client_finished_len;

    if(memcmp(d, s->s3->previous_server_finished,
	      s->s3->previous_server_finished_len))
        {
        SSLerr(SSL_F_SSL_PARSE_SERVERHELLO_RENEGOTIATE_EXT,SSL_R_RENEGOTIATION_MISMATCH);
        *al=SSL_AD_ILLEGAL_PARAMETER;
        return 0;
        }
#ifdef OPENSSL_RI_DEBUG
    fprintf(stderr, "%s RI extension received by client\n",
				ilen ? "Non-empty" : "Empty");
#endif
    s->s3->send_connection_binding=1;

    return 1;
    }