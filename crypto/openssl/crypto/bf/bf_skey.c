
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
#include <string.h>
#include <openssl/crypto.h>
#include <openssl/blowfish.h>
#include "bf_locl.h"
#include "bf_pi.h"

void BF_set_key(BF_KEY *key, int len, const unsigned char *data)
#ifdef OPENSSL_FIPS
	{
	fips_cipher_abort(BLOWFISH);
	private_BF_set_key(key, len, data);
	}
void private_BF_set_key(BF_KEY *key, int len, const unsigned char *data)
#endif
	{
	int i;
	BF_LONG *p,ri,in[2];
	const unsigned char *d,*end;


	memcpy(key,&bf_init,sizeof(BF_KEY));
	p=key->P;

	if (len > ((BF_ROUNDS+2)*4)) len=(BF_ROUNDS+2)*4;

	d=data;
	end= &(data[len]);
	for (i=0; i<(BF_ROUNDS+2); i++)
		{
		ri= *(d++);
		if (d >= end) d=data;

		ri<<=8;
		ri|= *(d++);
		if (d >= end) d=data;

		ri<<=8;
		ri|= *(d++);
		if (d >= end) d=data;

		ri<<=8;
		ri|= *(d++);
		if (d >= end) d=data;

		p[i]^=ri;
		}

	in[0]=0L;
	in[1]=0L;
	for (i=0; i<(BF_ROUNDS+2); i+=2)
		{
		BF_encrypt(in,key);
		p[i  ]=in[0];
		p[i+1]=in[1];
		}

	p=key->S;
	for (i=0; i<4*256; i+=2)
		{
		BF_encrypt(in,key);
		p[i  ]=in[0];
		p[i+1]=in[1];
		}
	}