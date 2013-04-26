
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
#include <time.h>
#include "cryptlib.h"
#include "o_time.h"
#include <openssl/asn1.h>

#if 0
int i2d_ASN1_UTCTIME(ASN1_UTCTIME *a, unsigned char **pp)
	{
#ifndef CHARSET_EBCDIC
	return(i2d_ASN1_bytes((ASN1_STRING *)a,pp,
		V_ASN1_UTCTIME,V_ASN1_UNIVERSAL));
#else
	/* KLUDGE! We convert to ascii before writing DER */
	int len;
	char tmp[24];
	ASN1_STRING x = *(ASN1_STRING *)a;

	len = x.length;
	ebcdic2ascii(tmp, x.data, (len >= sizeof tmp) ? sizeof tmp : len);
	x.data = tmp;
	return i2d_ASN1_bytes(&x, pp, V_ASN1_UTCTIME,V_ASN1_UNIVERSAL);
#endif
	}


ASN1_UTCTIME *d2i_ASN1_UTCTIME(ASN1_UTCTIME **a, unsigned char **pp,
	     long length)
	{
	ASN1_UTCTIME *ret=NULL;

	ret=(ASN1_UTCTIME *)d2i_ASN1_bytes((ASN1_STRING **)a,pp,length,
		V_ASN1_UTCTIME,V_ASN1_UNIVERSAL);
	if (ret == NULL)
		{
		ASN1err(ASN1_F_D2I_ASN1_UTCTIME,ERR_R_NESTED_ASN1_ERROR);
		return(NULL);
		}
#ifdef CHARSET_EBCDIC
	ascii2ebcdic(ret->data, ret->data, ret->length);
#endif
	if (!ASN1_UTCTIME_check(ret))
		{
		ASN1err(ASN1_F_D2I_ASN1_UTCTIME,ASN1_R_INVALID_TIME_FORMAT);
		goto err;
		}

	return(ret);
err:
	if ((ret != NULL) && ((a == NULL) || (*a != ret)))
		M_ASN1_UTCTIME_free(ret);
	return(NULL);
	}

#endif

int ASN1_UTCTIME_check(ASN1_UTCTIME *d)
	{
	static const int min[8]={ 0, 1, 1, 0, 0, 0, 0, 0};
	static const int max[8]={99,12,31,23,59,59,12,59};
	char *a;
	int n,i,l,o;

	if (d->type != V_ASN1_UTCTIME) return(0);
	l=d->length;
	a=(char *)d->data;
	o=0;

	if (l < 11) goto err;
	for (i=0; i<6; i++)
		{
		if ((i == 5) && ((a[o] == 'Z') ||
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
	if (a[o] == 'Z')
		o++;
	else if ((a[o] == '+') || (a[o] == '-'))
		{
		o++;
		if (o+4 > l) goto err;
		for (i=6; i<8; i++)
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
	return(o == l);
err:
	return(0);
	}

int ASN1_UTCTIME_set_string(ASN1_UTCTIME *s, const char *str)
	{
	ASN1_UTCTIME t;

	t.type=V_ASN1_UTCTIME;
	t.length=strlen(str);
	t.data=(unsigned char *)str;
	if (ASN1_UTCTIME_check(&t))
		{
		if (s != NULL)
			{
			if (!ASN1_STRING_set((ASN1_STRING *)s,
				(unsigned char *)str,t.length))
				return 0;
			s->type = V_ASN1_UTCTIME;
			}
		return(1);
		}
	else
		return(0);
	}

ASN1_UTCTIME *ASN1_UTCTIME_set(ASN1_UTCTIME *s, time_t t)
	{
	return ASN1_UTCTIME_adj(s, t, 0, 0);
	}

ASN1_UTCTIME *ASN1_UTCTIME_adj(ASN1_UTCTIME *s, time_t t,
				int offset_day, long offset_sec)
	{
	char *p;
	struct tm *ts;
	struct tm data;
	size_t len = 20;

	if (s == NULL)
		s=M_ASN1_UTCTIME_new();
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

	if((ts->tm_year < 50) || (ts->tm_year >= 150))
		return NULL;

	p=(char *)s->data;
	if ((p == NULL) || ((size_t)s->length < len))
		{
		p=OPENSSL_malloc(len);
		if (p == NULL)
			{
			ASN1err(ASN1_F_ASN1_UTCTIME_ADJ,ERR_R_MALLOC_FAILURE);
			return(NULL);
			}
		if (s->data != NULL)
			OPENSSL_free(s->data);
		s->data=(unsigned char *)p;
		}

	BIO_snprintf(p,len,"%02d%02d%02d%02d%02d%02dZ",ts->tm_year%100,
		     ts->tm_mon+1,ts->tm_mday,ts->tm_hour,ts->tm_min,ts->tm_sec);
	s->length=strlen(p);
	s->type=V_ASN1_UTCTIME;
#ifdef CHARSET_EBCDIC_not
	ebcdic2ascii(s->data, s->data, s->length);
#endif
	return(s);
	}


int ASN1_UTCTIME_cmp_time_t(const ASN1_UTCTIME *s, time_t t)
	{
	struct tm *tm;
	struct tm data;
	int offset;
	int year;

#define g2(p) (((p)[0]-'0')*10+(p)[1]-'0')

	if (s->data[12] == 'Z')
		offset=0;
	else
		{
		offset = g2(s->data+13)*60+g2(s->data+15);
		if (s->data[12] == '-')
			offset = -offset;
		}

	t -= offset*60; /* FIXME: may overflow in extreme cases */

	tm = OPENSSL_gmtime(&t, &data);
	
#define return_cmp(a,b) if ((a)<(b)) return -1; else if ((a)>(b)) return 1
	year = g2(s->data);
	if (year < 50)
		year += 100;
	return_cmp(year,              tm->tm_year);
	return_cmp(g2(s->data+2) - 1, tm->tm_mon);
	return_cmp(g2(s->data+4),     tm->tm_mday);
	return_cmp(g2(s->data+6),     tm->tm_hour);
	return_cmp(g2(s->data+8),     tm->tm_min);
	return_cmp(g2(s->data+10),    tm->tm_sec);
#undef g2
#undef return_cmp

	return 0;
	}


#if 0
time_t ASN1_UTCTIME_get(const ASN1_UTCTIME *s)
	{
	struct tm tm;
	int offset;

	memset(&tm,'\0',sizeof tm);

#define g2(p) (((p)[0]-'0')*10+(p)[1]-'0')
	tm.tm_year=g2(s->data);
	if(tm.tm_year < 50)
		tm.tm_year+=100;
	tm.tm_mon=g2(s->data+2)-1;
	tm.tm_mday=g2(s->data+4);
	tm.tm_hour=g2(s->data+6);
	tm.tm_min=g2(s->data+8);
	tm.tm_sec=g2(s->data+10);
	if(s->data[12] == 'Z')
		offset=0;
	else
		{
		offset=g2(s->data+13)*60+g2(s->data+15);
		if(s->data[12] == '-')
			offset= -offset;
		}
#undef g2

	return mktime(&tm)-offset*60; /* FIXME: mktime assumes the current timezone
	                               * instead of UTC, and unless we rewrite OpenSSL
				       * in Lisp we cannot locally change the timezone
				       * without possibly interfering with other parts
	                               * of the program. timegm, which uses UTC, is
				       * non-standard.
	                               * Also time_t is inappropriate for general
	                               * UTC times because it may a 32 bit type. */
	}
#endif