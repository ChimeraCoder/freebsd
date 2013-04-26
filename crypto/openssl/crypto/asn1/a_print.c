
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
#include <openssl/asn1.h>

int ASN1_PRINTABLE_type(const unsigned char *s, int len)
	{
	int c;
	int ia5=0;
	int t61=0;

	if (len <= 0) len= -1;
	if (s == NULL) return(V_ASN1_PRINTABLESTRING);

	while ((*s) && (len-- != 0))
		{
		c= *(s++);
#ifndef CHARSET_EBCDIC
		if (!(	((c >= 'a') && (c <= 'z')) ||
			((c >= 'A') && (c <= 'Z')) ||
			(c == ' ') ||
			((c >= '0') && (c <= '9')) ||
			(c == ' ') || (c == '\'') ||
			(c == '(') || (c == ')') ||
			(c == '+') || (c == ',') ||
			(c == '-') || (c == '.') ||
			(c == '/') || (c == ':') ||
			(c == '=') || (c == '?')))
			ia5=1;
		if (c&0x80)
			t61=1;
#else
		if (!isalnum(c) && (c != ' ') &&
		    strchr("'()+,-./:=?", c) == NULL)
			ia5=1;
		if (os_toascii[c] & 0x80)
			t61=1;
#endif
		}
	if (t61) return(V_ASN1_T61STRING);
	if (ia5) return(V_ASN1_IA5STRING);
	return(V_ASN1_PRINTABLESTRING);
	}

int ASN1_UNIVERSALSTRING_to_string(ASN1_UNIVERSALSTRING *s)
	{
	int i;
	unsigned char *p;

	if (s->type != V_ASN1_UNIVERSALSTRING) return(0);
	if ((s->length%4) != 0) return(0);
	p=s->data;
	for (i=0; i<s->length; i+=4)
		{
		if ((p[0] != '\0') || (p[1] != '\0') || (p[2] != '\0'))
			break;
		else
			p+=4;
		}
	if (i < s->length) return(0);
	p=s->data;
	for (i=3; i<s->length; i+=4)
		{
		*(p++)=s->data[i];
		}
	*(p)='\0';
	s->length/=4;
	s->type=ASN1_PRINTABLE_type(s->data,s->length);
	return(1);
	}