
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
#include "cryptlib.h"
#include <openssl/asn1t.h>

int i2d_ASN1_BOOLEAN(int a, unsigned char **pp)
	{
	int r;
	unsigned char *p;

	r=ASN1_object_size(0,1,V_ASN1_BOOLEAN);
	if (pp == NULL) return(r);
	p= *pp;

	ASN1_put_object(&p,0,1,V_ASN1_BOOLEAN,V_ASN1_UNIVERSAL);
	*(p++)= (unsigned char)a;
	*pp=p;
	return(r);
	}

int d2i_ASN1_BOOLEAN(int *a, const unsigned char **pp, long length)
	{
	int ret= -1;
	const unsigned char *p;
	long len;
	int inf,tag,xclass;
	int i=0;

	p= *pp;
	inf=ASN1_get_object(&p,&len,&tag,&xclass,length);
	if (inf & 0x80)
		{
		i=ASN1_R_BAD_OBJECT_HEADER;
		goto err;
		}

	if (tag != V_ASN1_BOOLEAN)
		{
		i=ASN1_R_EXPECTING_A_BOOLEAN;
		goto err;
		}

	if (len != 1)
		{
		i=ASN1_R_BOOLEAN_IS_WRONG_LENGTH;
		goto err;
		}
	ret= (int)*(p++);
	if (a != NULL) (*a)=ret;
	*pp=p;
	return(ret);
err:
	ASN1err(ASN1_F_D2I_ASN1_BOOLEAN,i);
	return(ret);
	}