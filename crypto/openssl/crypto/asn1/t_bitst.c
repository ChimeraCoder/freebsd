
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
#include <openssl/conf.h>
#include <openssl/x509v3.h>

int ASN1_BIT_STRING_name_print(BIO *out, ASN1_BIT_STRING *bs,
				BIT_STRING_BITNAME *tbl, int indent)
{
	BIT_STRING_BITNAME *bnam;
	char first = 1;
	BIO_printf(out, "%*s", indent, "");
	for(bnam = tbl; bnam->lname; bnam++) {
		if(ASN1_BIT_STRING_get_bit(bs, bnam->bitnum)) {
			if(!first) BIO_puts(out, ", ");
			BIO_puts(out, bnam->lname);
			first = 0;
		}
	}
	BIO_puts(out, "\n");
	return 1;
}

int ASN1_BIT_STRING_set_asc(ASN1_BIT_STRING *bs, char *name, int value,
				BIT_STRING_BITNAME *tbl)
{
	int bitnum;
	bitnum = ASN1_BIT_STRING_num_asc(name, tbl);
	if(bitnum < 0) return 0;
	if(bs) {
		if(!ASN1_BIT_STRING_set_bit(bs, bitnum, value))
			return 0;
	}
	return 1;
}

int ASN1_BIT_STRING_num_asc(char *name, BIT_STRING_BITNAME *tbl)
{
	BIT_STRING_BITNAME *bnam;
	for(bnam = tbl; bnam->lname; bnam++) {
		if(!strcmp(bnam->sname, name) ||
			!strcmp(bnam->lname, name) ) return bnam->bitnum;
	}
	return -1;
}