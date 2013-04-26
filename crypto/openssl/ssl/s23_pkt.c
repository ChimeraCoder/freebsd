
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
#include <errno.h>
#define USE_SOCKETS
#include "ssl_locl.h"
#include <openssl/evp.h>
#include <openssl/buffer.h>

int ssl23_write_bytes(SSL *s)
	{
	int i,num,tot;
	char *buf;

	buf=s->init_buf->data;
	tot=s->init_off;
	num=s->init_num;
	for (;;)
		{
		s->rwstate=SSL_WRITING;
		i=BIO_write(s->wbio,&(buf[tot]),num);
		if (i <= 0)
			{
			s->init_off=tot;
			s->init_num=num;
			return(i);
			}
		s->rwstate=SSL_NOTHING;
		if (i == num) return(tot+i);

		num-=i;
		tot+=i;
		}
	}

/* return regularly only when we have read (at least) 'n' bytes */
int ssl23_read_bytes(SSL *s, int n)
	{
	unsigned char *p;
	int j;

	if (s->packet_length < (unsigned int)n)
		{
		p=s->packet;

		for (;;)
			{
			s->rwstate=SSL_READING;
			j=BIO_read(s->rbio,(char *)&(p[s->packet_length]),
				n-s->packet_length);
			if (j <= 0)
				return(j);
			s->rwstate=SSL_NOTHING;
			s->packet_length+=j;
			if (s->packet_length >= (unsigned int)n)
				return(s->packet_length);
			}
		}
	return(n);
	}