
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
#include <openssl/bn.h>
#include <openssl/asn1.h>
#include <openssl/dh.h>
#include <openssl/pem.h>

unsigned char data[]={
0xD4,0xA0,0xBA,0x02,0x50,0xB6,0xFD,0x2E,
0xC6,0x26,0xE7,0xEF,0xD6,0x37,0xDF,0x76,
0xC7,0x16,0xE2,0x2D,0x09,0x44,0xB8,0x8B,
	};

main()
	{
	DH *dh;

	dh=DH_new();
	dh->p=BN_bin2bn(data,sizeof(data),NULL);
	dh->g=BN_new();
	BN_set_word(dh->g,3);
	PEM_write_DHparams(stdout,dh);
	}