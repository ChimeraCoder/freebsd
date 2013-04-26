
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

#include <openssl/cast.h>
#include "cast_lcl.h"
#include <openssl/opensslv.h>

const char CAST_version[]="CAST" OPENSSL_VERSION_PTEXT;

void CAST_ecb_encrypt(const unsigned char *in, unsigned char *out,
		      const CAST_KEY *ks, int enc)
	{
	CAST_LONG l,d[2];

	n2l(in,l); d[0]=l;
	n2l(in,l); d[1]=l;
	if (enc)
		CAST_encrypt(d,ks);
	else
		CAST_decrypt(d,ks);
	l=d[0]; l2n(l,out);
	l=d[1]; l2n(l,out);
	l=d[0]=d[1]=0;
	}