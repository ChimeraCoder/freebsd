
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

#include <openssl/crypto.h>
#include "des_locl.h"

void DES_string_to_key(const char *str, DES_cblock *key)
	{
	DES_key_schedule ks;
	int i,length;
	register unsigned char j;

	memset(key,0,8);
	length=strlen(str);
#ifdef OLD_STR_TO_KEY
	for (i=0; i<length; i++)
		(*key)[i%8]^=(str[i]<<1);
#else /* MIT COMPATIBLE */
	for (i=0; i<length; i++)
		{
		j=str[i];
		if ((i%16) < 8)
			(*key)[i%8]^=(j<<1);
		else
			{
			/* Reverse the bit order 05/05/92 eay */
			j=((j<<4)&0xf0)|((j>>4)&0x0f);
			j=((j<<2)&0xcc)|((j>>2)&0x33);
			j=((j<<1)&0xaa)|((j>>1)&0x55);
			(*key)[7-(i%8)]^=j;
			}
		}
#endif
	DES_set_odd_parity(key);
#ifdef EXPERIMENTAL_STR_TO_STRONG_KEY
	if(DES_is_weak_key(key))
	    (*key)[7] ^= 0xF0;
	DES_set_key(key,&ks);
#else
	DES_set_key_unchecked(key,&ks);
#endif
	DES_cbc_cksum((const unsigned char*)str,key,length,&ks,key);
	OPENSSL_cleanse(&ks,sizeof(ks));
	DES_set_odd_parity(key);
	}

void DES_string_to_2keys(const char *str, DES_cblock *key1, DES_cblock *key2)
	{
	DES_key_schedule ks;
	int i,length;
	register unsigned char j;

	memset(key1,0,8);
	memset(key2,0,8);
	length=strlen(str);
#ifdef OLD_STR_TO_KEY
	if (length <= 8)
		{
		for (i=0; i<length; i++)
			{
			(*key2)[i]=(*key1)[i]=(str[i]<<1);
			}
		}
	else
		{
		for (i=0; i<length; i++)
			{
			if ((i/8)&1)
				(*key2)[i%8]^=(str[i]<<1);
			else
				(*key1)[i%8]^=(str[i]<<1);
			}
		}
#else /* MIT COMPATIBLE */
	for (i=0; i<length; i++)
		{
		j=str[i];
		if ((i%32) < 16)
			{
			if ((i%16) < 8)
				(*key1)[i%8]^=(j<<1);
			else
				(*key2)[i%8]^=(j<<1);
			}
		else
			{
			j=((j<<4)&0xf0)|((j>>4)&0x0f);
			j=((j<<2)&0xcc)|((j>>2)&0x33);
			j=((j<<1)&0xaa)|((j>>1)&0x55);
			if ((i%16) < 8)
				(*key1)[7-(i%8)]^=j;
			else
				(*key2)[7-(i%8)]^=j;
			}
		}
	if (length <= 8) memcpy(key2,key1,8);
#endif
	DES_set_odd_parity(key1);
	DES_set_odd_parity(key2);
#ifdef EXPERIMENTAL_STR_TO_STRONG_KEY
	if(DES_is_weak_key(key1))
	    (*key1)[7] ^= 0xF0;
	DES_set_key(key1,&ks);
#else
	DES_set_key_unchecked(key1,&ks);
#endif
	DES_cbc_cksum((const unsigned char*)str,key1,length,&ks,key1);
#ifdef EXPERIMENTAL_STR_TO_STRONG_KEY
	if(DES_is_weak_key(key2))
	    (*key2)[7] ^= 0xF0;
	DES_set_key(key2,&ks);
#else
	DES_set_key_unchecked(key2,&ks);
#endif
	DES_cbc_cksum((const unsigned char*)str,key2,length,&ks,key2);
	OPENSSL_cleanse(&ks,sizeof(ks));
	DES_set_odd_parity(key1);
	DES_set_odd_parity(key2);
	}