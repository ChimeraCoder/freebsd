
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

#include <openssl/rc4.h>
#include "rc4_locl.h"
#include <openssl/opensslv.h>

const char RC4_version[]="RC4" OPENSSL_VERSION_PTEXT;

const char *RC4_options(void)
	{
#ifdef RC4_INDEX
	if (sizeof(RC4_INT) == 1)
		return("rc4(idx,char)");
	else
		return("rc4(idx,int)");
#else
	if (sizeof(RC4_INT) == 1)
		return("rc4(ptr,char)");
	else
		return("rc4(ptr,int)");
#endif
	}

/* RC4 as implemented from a posting from
 * Newsgroups: sci.crypt
 * From: sterndark@netcom.com (David Sterndark)
 * Subject: RC4 Algorithm revealed.
 * Message-ID: <sternCvKL4B.Hyy@netcom.com>
 * Date: Wed, 14 Sep 1994 06:35:31 GMT
 */

void private_RC4_set_key(RC4_KEY *key, int len, const unsigned char *data)
	{
        register RC4_INT tmp;
        register int id1,id2;
        register RC4_INT *d;
        unsigned int i;
        
        d= &(key->data[0]);
        key->x = 0;     
        key->y = 0;     
        id1=id2=0;     

#define SK_LOOP(d,n) { \
		tmp=d[(n)]; \
		id2 = (data[id1] + tmp + id2) & 0xff; \
		if (++id1 == len) id1=0; \
		d[(n)]=d[id2]; \
		d[id2]=tmp; }

	for (i=0; i < 256; i++) d[i]=i;
	for (i=0; i < 256; i+=4)
		{
		SK_LOOP(d,i+0);
		SK_LOOP(d,i+1);
		SK_LOOP(d,i+2);
		SK_LOOP(d,i+3);
		}
	}
    