
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
#include <openssl/buffer.h>
#include <openssl/objects.h>
#include <openssl/asn1.h>

static int asn1_print_info(BIO *bp, int tag, int xclass,int constructed,
	int indent);
static int asn1_parse2(BIO *bp, const unsigned char **pp, long length,
	int offset, int depth, int indent, int dump);
static int asn1_print_info(BIO *bp, int tag, int xclass, int constructed,
	     int indent)
	{
	static const char fmt[]="%-18s";
	char str[128];
	const char *p;

	if (constructed & V_ASN1_CONSTRUCTED)
		p="cons: ";
	else
		p="prim: ";
	if (BIO_write(bp,p,6) < 6) goto err;
	BIO_indent(bp,indent,128);

	p=str;
	if ((xclass & V_ASN1_PRIVATE) == V_ASN1_PRIVATE)
		BIO_snprintf(str,sizeof str,"priv [ %d ] ",tag);
	else if ((xclass & V_ASN1_CONTEXT_SPECIFIC) == V_ASN1_CONTEXT_SPECIFIC)
		BIO_snprintf(str,sizeof str,"cont [ %d ]",tag);
	else if ((xclass & V_ASN1_APPLICATION) == V_ASN1_APPLICATION)
		BIO_snprintf(str,sizeof str,"appl [ %d ]",tag);
	else if (tag > 30)
		BIO_snprintf(str,sizeof str,"<ASN1 %d>",tag);
	else
		p = ASN1_tag2str(tag);

	if (BIO_printf(bp,fmt,p) <= 0)
		goto err;
	return(1);
err:
	return(0);
	}

int ASN1_parse(BIO *bp, const unsigned char *pp, long len, int indent)
	{
	return(asn1_parse2(bp,&pp,len,0,0,indent,0));
	}

int ASN1_parse_dump(BIO *bp, const unsigned char *pp, long len, int indent, int dump)
	{
	return(asn1_parse2(bp,&pp,len,0,0,indent,dump));
	}

static int asn1_parse2(BIO *bp, const unsigned char **pp, long length, int offset,
	     int depth, int indent, int dump)
	{
	const unsigned char *p,*ep,*tot,*op,*opp;
	long len;
	int tag,xclass,ret=0;
	int nl,hl,j,r;
	ASN1_OBJECT *o=NULL;
	ASN1_OCTET_STRING *os=NULL;
	/* ASN1_BMPSTRING *bmp=NULL;*/
	int dump_indent;

#if 0
	dump_indent = indent;
#else
	dump_indent = 6;	/* Because we know BIO_dump_indent() */
#endif
	p= *pp;
	tot=p+length;
	op=p-1;
	while ((p < tot) && (op < p))
		{
		op=p;
		j=ASN1_get_object(&p,&len,&tag,&xclass,length);
#ifdef LINT
		j=j;
#endif
		if (j & 0x80)
			{
			if (BIO_write(bp,"Error in encoding\n",18) <= 0)
				goto end;
			ret=0;
			goto end;
			}
		hl=(p-op);
		length-=hl;
		/* if j == 0x21 it is a constructed indefinite length object */
		if (BIO_printf(bp,"%5ld:",(long)offset+(long)(op- *pp))
			<= 0) goto end;

		if (j != (V_ASN1_CONSTRUCTED | 1))
			{
			if (BIO_printf(bp,"d=%-2d hl=%ld l=%4ld ",
				depth,(long)hl,len) <= 0)
				goto end;
			}
		else
			{
			if (BIO_printf(bp,"d=%-2d hl=%ld l=inf  ",
				depth,(long)hl) <= 0)
				goto end;
			}
		if (!asn1_print_info(bp,tag,xclass,j,(indent)?depth:0))
			goto end;
		if (j & V_ASN1_CONSTRUCTED)
			{
			ep=p+len;
			if (BIO_write(bp,"\n",1) <= 0) goto end;
			if (len > length)
				{
				BIO_printf(bp,
					"length is greater than %ld\n",length);
				ret=0;
				goto end;
				}
			if ((j == 0x21) && (len == 0))
				{
				for (;;)
					{
					r=asn1_parse2(bp,&p,(long)(tot-p),
						offset+(p - *pp),depth+1,
						indent,dump);
					if (r == 0) { ret=0; goto end; }
					if ((r == 2) || (p >= tot)) break;
					}
				}
			else
				while (p < ep)
					{
					r=asn1_parse2(bp,&p,(long)len,
						offset+(p - *pp),depth+1,
						indent,dump);
					if (r == 0) { ret=0; goto end; }
					}
			}
		else if (xclass != 0)
			{
			p+=len;
			if (BIO_write(bp,"\n",1) <= 0) goto end;
			}
		else
			{
			nl=0;
			if (	(tag == V_ASN1_PRINTABLESTRING) ||
				(tag == V_ASN1_T61STRING) ||
				(tag == V_ASN1_IA5STRING) ||
				(tag == V_ASN1_VISIBLESTRING) ||
				(tag == V_ASN1_NUMERICSTRING) ||
				(tag == V_ASN1_UTF8STRING) ||
				(tag == V_ASN1_UTCTIME) ||
				(tag == V_ASN1_GENERALIZEDTIME))
				{
				if (BIO_write(bp,":",1) <= 0) goto end;
				if ((len > 0) &&
					BIO_write(bp,(const char *)p,(int)len)
					!= (int)len)
					goto end;
				}
			else if (tag == V_ASN1_OBJECT)
				{
				opp=op;
				if (d2i_ASN1_OBJECT(&o,&opp,len+hl) != NULL)
					{
					if (BIO_write(bp,":",1) <= 0) goto end;
					i2a_ASN1_OBJECT(bp,o);
					}
				else
					{
					if (BIO_write(bp,":BAD OBJECT",11) <= 0)
						goto end;
					}
				}
			else if (tag == V_ASN1_BOOLEAN)
				{
				int ii;

				opp=op;
				ii=d2i_ASN1_BOOLEAN(NULL,&opp,len+hl);
				if (ii < 0)
					{
					if (BIO_write(bp,"Bad boolean\n",12) <= 0)
						goto end;
					}
				BIO_printf(bp,":%d",ii);
				}
			else if (tag == V_ASN1_BMPSTRING)
				{
				/* do the BMP thang */
				}
			else if (tag == V_ASN1_OCTET_STRING)
				{
				int i,printable=1;

				opp=op;
				os=d2i_ASN1_OCTET_STRING(NULL,&opp,len+hl);
				if (os != NULL && os->length > 0)
					{
					opp = os->data;
					/* testing whether the octet string is
					 * printable */
					for (i=0; i<os->length; i++)
						{
						if ((	(opp[i] < ' ') &&
							(opp[i] != '\n') &&
							(opp[i] != '\r') &&
							(opp[i] != '\t')) ||
							(opp[i] > '~'))
							{
							printable=0;
							break;
							}
						}
					if (printable)
					/* printable string */
						{
						if (BIO_write(bp,":",1) <= 0)
							goto end;
						if (BIO_write(bp,(const char *)opp,
							os->length) <= 0)
							goto end;
						}
					else if (!dump)
					/* not printable => print octet string
					 * as hex dump */
						{
						if (BIO_write(bp,"[HEX DUMP]:",11) <= 0)
							goto end;
						for (i=0; i<os->length; i++)
							{
							if (BIO_printf(bp,"%02X"
								, opp[i]) <= 0)
								goto end;
							}
						}
					else
					/* print the normal dump */
						{
						if (!nl) 
							{
							if (BIO_write(bp,"\n",1) <= 0)
								goto end;
							}
						if (BIO_dump_indent(bp,
							(const char *)opp,
							((dump == -1 || dump > 
							os->length)?os->length:dump),
							dump_indent) <= 0)
							goto end;
						nl=1;
						}
					}
				if (os != NULL)
					{
					M_ASN1_OCTET_STRING_free(os);
					os=NULL;
					}
				}
			else if (tag == V_ASN1_INTEGER)
				{
				ASN1_INTEGER *bs;
				int i;

				opp=op;
				bs=d2i_ASN1_INTEGER(NULL,&opp,len+hl);
				if (bs != NULL)
					{
					if (BIO_write(bp,":",1) <= 0) goto end;
					if (bs->type == V_ASN1_NEG_INTEGER)
						if (BIO_write(bp,"-",1) <= 0)
							goto end;
					for (i=0; i<bs->length; i++)
						{
						if (BIO_printf(bp,"%02X",
							bs->data[i]) <= 0)
							goto end;
						}
					if (bs->length == 0)
						{
						if (BIO_write(bp,"00",2) <= 0)
							goto end;
						}
					}
				else
					{
					if (BIO_write(bp,"BAD INTEGER",11) <= 0)
						goto end;
					}
				M_ASN1_INTEGER_free(bs);
				}
			else if (tag == V_ASN1_ENUMERATED)
				{
				ASN1_ENUMERATED *bs;
				int i;

				opp=op;
				bs=d2i_ASN1_ENUMERATED(NULL,&opp,len+hl);
				if (bs != NULL)
					{
					if (BIO_write(bp,":",1) <= 0) goto end;
					if (bs->type == V_ASN1_NEG_ENUMERATED)
						if (BIO_write(bp,"-",1) <= 0)
							goto end;
					for (i=0; i<bs->length; i++)
						{
						if (BIO_printf(bp,"%02X",
							bs->data[i]) <= 0)
							goto end;
						}
					if (bs->length == 0)
						{
						if (BIO_write(bp,"00",2) <= 0)
							goto end;
						}
					}
				else
					{
					if (BIO_write(bp,"BAD ENUMERATED",11) <= 0)
						goto end;
					}
				M_ASN1_ENUMERATED_free(bs);
				}
			else if (len > 0 && dump)
				{
				if (!nl) 
					{
					if (BIO_write(bp,"\n",1) <= 0)
						goto end;
					}
				if (BIO_dump_indent(bp,(const char *)p,
					((dump == -1 || dump > len)?len:dump),
					dump_indent) <= 0)
					goto end;
				nl=1;
				}

			if (!nl) 
				{
				if (BIO_write(bp,"\n",1) <= 0) goto end;
				}
			p+=len;
			if ((tag == V_ASN1_EOC) && (xclass == 0))
				{
				ret=2; /* End of sequence */
				goto end;
				}
			}
		length-=len;
		}
	ret=1;
end:
	if (o != NULL) ASN1_OBJECT_free(o);
	if (os != NULL) M_ASN1_OCTET_STRING_free(os);
	*pp=p;
	return(ret);
	}

const char *ASN1_tag2str(int tag)
{
	static const char * const tag2str[] = {
	 "EOC", "BOOLEAN", "INTEGER", "BIT STRING", "OCTET STRING", /* 0-4 */
	 "NULL", "OBJECT", "OBJECT DESCRIPTOR", "EXTERNAL", "REAL", /* 5-9 */
	 "ENUMERATED", "<ASN1 11>", "UTF8STRING", "<ASN1 13>", 	    /* 10-13 */
	"<ASN1 14>", "<ASN1 15>", "SEQUENCE", "SET", 		    /* 15-17 */
	"NUMERICSTRING", "PRINTABLESTRING", "T61STRING",	    /* 18-20 */
	"VIDEOTEXSTRING", "IA5STRING", "UTCTIME","GENERALIZEDTIME", /* 21-24 */
	"GRAPHICSTRING", "VISIBLESTRING", "GENERALSTRING",	    /* 25-27 */
	"UNIVERSALSTRING", "<ASN1 29>", "BMPSTRING"		    /* 28-30 */
	};

	if((tag == V_ASN1_NEG_INTEGER) || (tag == V_ASN1_NEG_ENUMERATED))
							tag &= ~0x100;

	if(tag < 0 || tag > 30) return "(unknown)";
	return tag2str[tag];
}