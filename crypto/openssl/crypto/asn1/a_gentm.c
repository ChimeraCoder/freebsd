
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

/* GENERALIZEDTIME implementation, written by Steve Henson. Based on UTCTIME */

#include <stdio.h>
#include <time.h>
#include "cryptlib.h"
#include "o_time.h"
#include <openssl/asn1.h>

#if 0

int i2d_ASN1_GENERALIZEDTIME(ASN1_GENERALIZEDTIME *a, unsigned char **pp)
	{
#ifdef CHARSET_EBCDIC
	/* KLUDGE! We convert to ascii before writing DER */
	int len;
	char tmp[24];
	ASN1_STRING tmpstr = *(ASN1_STRING *)a;

	len = tmpstr.length;
	ebcdic2ascii(tmp, tmpstr.data, (len >= sizeof tmp) ? sizeof tmp : len);
	tmpstr.data = tmp;

	a = (ASN1_GENERALIZEDTIME *) &tmpstr;
#endif
	return(i2d_ASN1_bytes((ASN1_STRING *)a,pp,
		V_ASN1_GENERALIZEDTIME,V_ASN1_UNIVERSAL));
	}


ASN1_GENERALIZEDTIME *d2i_ASN1_GENERALIZEDTIME(ASN1_GENERALIZEDTIME **a,
	     unsigned char **pp, long length)
	{
	ASN1_GENERALIZEDTIME *ret=NULL;

	ret=(ASN1_GENERALIZEDTIME *)d2i_ASN1_bytes((ASN1_STRING **)a,pp,length,
		V_ASN1_GENERALIZEDTIME,V_ASN1_UNIVERSAL);
	if (ret == NULL)
		{
		ASN1err(ASN1_F_D2I_ASN1_GENERALIZEDTIME,ERR_R_NESTED_ASN1_ERROR);
		return(NULL);
		}
#ifdef CHARSET_EBCDIC
	ascii2ebcdic(ret->data, ret->data, ret->length);
#endif
	if (!ASN1_GENERALIZEDTIME_check(ret))
		{
		ASN1err(ASN1_F_D2I_ASN1_GENERALIZEDTIME,ASN1_R_INVALID_TIME_FORMAT);
		goto err;
		}

	return(ret);
err:
	if ((ret != NULL) && ((a == NULL) || (*a != ret)))
		M_ASN1_GENERALIZEDTIME_free(ret);
	return(NULL);
	}

#endif

int ASN1_GENERALIZEDTIME_check(ASN1_GENERALIZEDTIME *d)
	{
	static const int min[9]={ 0, 0, 1, 1, 0, 0, 0, 0, 0};
	static const int max[9]={99, 99,12,31,23,59,59,12,59};
	char *a;
	int n,i,l,o;

	if (d->type != V_ASN1_GENERALIZEDTIME) return(0);
	l=d->length;
	a=(char *)d->data;
	o=0;
	/* GENERALIZEDTIME is similar to UTCTIME except the year is
         * represented as YYYY. This stuff treats everything as a two digit
         * field so make first two fields 00 to 99
         */
	if (l < 13) goto err;
	for (i=0; i<7; i++)
		{
		if ((i == 6) && ((a[o] == 'Z') ||
			(a[o] == '+') || (a[o] == '-')))
			{ i++; break; }
		if ((a[o] < '0') || (a[o] > '9')) goto err;
		n= a[o]-'0';
		if (++o > l) goto err;

		if ((a[o] < '0') || (a[o] > '9')) goto err;
		n=(n*10)+ a[o]-'0';
		if (++o > l) goto err;

		if ((n < min[i]) || (n > max[i])) goto err;
		}
	/* Optional fractional seconds: decimal point followed by one
	 * or more digits.
	 */
	if (a[o] == '.')
		{
		if (++o > l) goto err;
		i = o;
		while ((a[o] >= '0') && (a[o] <= '9') && (o <= l))
			o++;
		/* Must have at least one digit after decimal point */
		if (i == o) goto err;
		}

	if (a[o] == 'Z')
		o++;
	else if ((a[o] == '+') || (a[o] == '-'))
		{
		o++;
		if (o+4 > l) goto err;
		for (i=7; i<9; i++)
			{
			if ((a[o] < '0') || (a[o] > '9')) goto err;
			n= a[o]-'0';
			o++;
			if ((a[o] < '0') || (a[o] > '9')) goto err;
			n=(n*10)+ a[o]-'0';
			if ((n < min[i]) || (n > max[i])) goto err;
			o++;
			}
		}
	else
		{
		/* Missing time zone information. */
		goto err;
		}
	return(o == l);
err:
	return(0);
	}

int ASN1_GENERALIZEDTIME_set_string(ASN1_GENERALIZEDTIME *s, const char *str)
	{
	ASN1_GENERALIZEDTIME t;

	t.type=V_ASN1_GENERALIZEDTIME;
	t.length=strlen(str);
	t.data=(unsigned char *)str;
	if (ASN1_GENERALIZEDTIME_check(&t))
		{
		if (s != NULL)
			{
			if (!ASN1_STRING_set((ASN1_STRING *)s,
				(unsigned char *)str,t.length))
				return 0;
			s->type=V_ASN1_GENERALIZEDTIME;
			}
		return(1);
		}
	else
		return(0);
	}

ASN1_GENERALIZEDTIME *ASN1_GENERALIZEDTIME_set(ASN1_GENERALIZEDTIME *s,
	     time_t t)
	{
		return ASN1_GENERALIZEDTIME_adj(s, t, 0, 0);
	}

ASN1_GENERALIZEDTIME *ASN1_GENERALIZEDTIME_adj(ASN1_GENERALIZEDTIME *s,
	     time_t t, int offset_day, long offset_sec)
	{
	char *p;
	struct tm *ts;
	struct tm data;
	size_t len = 20; 

	if (s == NULL)
		s=M_ASN1_GENERALIZEDTIME_new();
	if (s == NULL)
		return(NULL);

	ts=OPENSSL_gmtime(&t, &data);
	if (ts == NULL)
		return(NULL);

	if (offset_day || offset_sec)
		{ 
		if (!OPENSSL_gmtime_adj(ts, offset_day, offset_sec))
			return NULL;
		}

	p=(char *)s->data;
	if ((p == NULL) || ((size_t)s->length < len))
		{
		p=OPENSSL_malloc(len);
		if (p == NULL)
			{
			ASN1err(ASN1_F_ASN1_GENERALIZEDTIME_ADJ,
				ERR_R_MALLOC_FAILURE);
			return(NULL);
			}
		if (s->data != NULL)
			OPENSSL_free(s->data);
		s->data=(unsigned char *)p;
		}

	BIO_snprintf(p,len,"%04d%02d%02d%02d%02d%02dZ",ts->tm_year + 1900,
		     ts->tm_mon+1,ts->tm_mday,ts->tm_hour,ts->tm_min,ts->tm_sec);
	s->length=strlen(p);
	s->type=V_ASN1_GENERALIZEDTIME;
#ifdef CHARSET_EBCDIC_not
	ebcdic2ascii(s->data, s->data, s->length);
#endif
	return(s);
	}