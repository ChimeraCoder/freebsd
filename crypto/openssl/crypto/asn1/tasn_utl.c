
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


#include <stddef.h>
#include <string.h>
#include <openssl/asn1.h>
#include <openssl/asn1t.h>
#include <openssl/objects.h>
#include <openssl/err.h>

/* Utility functions for manipulating fields and offsets */

/* Add 'offset' to 'addr' */
#define offset2ptr(addr, offset) (void *)(((char *) addr) + offset)

/* Given an ASN1_ITEM CHOICE type return
 * the selector value
 */

int asn1_get_choice_selector(ASN1_VALUE **pval, const ASN1_ITEM *it)
	{
	int *sel = offset2ptr(*pval, it->utype);
	return *sel;
	}

/* Given an ASN1_ITEM CHOICE type set
 * the selector value, return old value.
 */

int asn1_set_choice_selector(ASN1_VALUE **pval, int value, const ASN1_ITEM *it)
	{	
	int *sel, ret;
	sel = offset2ptr(*pval, it->utype);
	ret = *sel;
	*sel = value;
	return ret;
	}

/* Do reference counting. The value 'op' decides what to do. 
 * if it is +1 then the count is incremented. If op is 0 count is
 * set to 1. If op is -1 count is decremented and the return value
 * is the current refrence count or 0 if no reference count exists.
 */

int asn1_do_lock(ASN1_VALUE **pval, int op, const ASN1_ITEM *it)
	{
	const ASN1_AUX *aux;
	int *lck, ret;
	if ((it->itype != ASN1_ITYPE_SEQUENCE)
	   && (it->itype != ASN1_ITYPE_NDEF_SEQUENCE))
		return 0;
	aux = it->funcs;
	if (!aux || !(aux->flags & ASN1_AFLG_REFCOUNT))
		return 0;
	lck = offset2ptr(*pval, aux->ref_offset);
	if (op == 0)
		{
		*lck = 1;
		return 1;
		}
	ret = CRYPTO_add(lck, op, aux->ref_lock);
#ifdef REF_PRINT
	fprintf(stderr, "%s: Reference Count: %d\n", it->sname, *lck);
#endif
#ifdef REF_CHECK
	if (ret < 0) 
		fprintf(stderr, "%s, bad reference count\n", it->sname);
#endif
	return ret;
	}

static ASN1_ENCODING *asn1_get_enc_ptr(ASN1_VALUE **pval, const ASN1_ITEM *it)
	{
	const ASN1_AUX *aux;
	if (!pval || !*pval)
		return NULL;
	aux = it->funcs;
	if (!aux || !(aux->flags & ASN1_AFLG_ENCODING))
		return NULL;
	return offset2ptr(*pval, aux->enc_offset);
	}

void asn1_enc_init(ASN1_VALUE **pval, const ASN1_ITEM *it)
	{
	ASN1_ENCODING *enc;
	enc = asn1_get_enc_ptr(pval, it);
	if (enc)
		{
		enc->enc = NULL;
		enc->len = 0;
		enc->modified = 1;
		}
	}

void asn1_enc_free(ASN1_VALUE **pval, const ASN1_ITEM *it)
	{
	ASN1_ENCODING *enc;
	enc = asn1_get_enc_ptr(pval, it);
	if (enc)
		{
		if (enc->enc)
			OPENSSL_free(enc->enc);
		enc->enc = NULL;
		enc->len = 0;
		enc->modified = 1;
		}
	}

int asn1_enc_save(ASN1_VALUE **pval, const unsigned char *in, int inlen,
							 const ASN1_ITEM *it)
	{
	ASN1_ENCODING *enc;
	enc = asn1_get_enc_ptr(pval, it);
	if (!enc)
		return 1;

	if (enc->enc)
		OPENSSL_free(enc->enc);
	enc->enc = OPENSSL_malloc(inlen);
	if (!enc->enc)
		return 0;
	memcpy(enc->enc, in, inlen);
	enc->len = inlen;
	enc->modified = 0;

	return 1;
	}
		
int asn1_enc_restore(int *len, unsigned char **out, ASN1_VALUE **pval,
							const ASN1_ITEM *it)
	{
	ASN1_ENCODING *enc;
	enc = asn1_get_enc_ptr(pval, it);
	if (!enc || enc->modified)
		return 0;
	if (out)
		{
		memcpy(*out, enc->enc, enc->len);
		*out += enc->len;
		}
	if (len)
		*len = enc->len;
	return 1;
	}

/* Given an ASN1_TEMPLATE get a pointer to a field */
ASN1_VALUE ** asn1_get_field_ptr(ASN1_VALUE **pval, const ASN1_TEMPLATE *tt)
	{
	ASN1_VALUE **pvaltmp;
	if (tt->flags & ASN1_TFLG_COMBINE)
		return pval;
	pvaltmp = offset2ptr(*pval, tt->offset);
	/* NOTE for BOOLEAN types the field is just a plain
 	 * int so we can't return int **, so settle for
	 * (int *).
	 */
	return pvaltmp;
	}

/* Handle ANY DEFINED BY template, find the selector, look up
 * the relevant ASN1_TEMPLATE in the table and return it.
 */

const ASN1_TEMPLATE *asn1_do_adb(ASN1_VALUE **pval, const ASN1_TEMPLATE *tt,
								int nullerr)
	{
	const ASN1_ADB *adb;
	const ASN1_ADB_TABLE *atbl;
	long selector;
	ASN1_VALUE **sfld;
	int i;
	if (!(tt->flags & ASN1_TFLG_ADB_MASK))
		return tt;

	/* Else ANY DEFINED BY ... get the table */
	adb = ASN1_ADB_ptr(tt->item);

	/* Get the selector field */
	sfld = offset2ptr(*pval, adb->offset);

	/* Check if NULL */
	if (!sfld)
		{
		if (!adb->null_tt)
			goto err;
		return adb->null_tt;
		}

	/* Convert type to a long:
	 * NB: don't check for NID_undef here because it
	 * might be a legitimate value in the table
	 */
	if (tt->flags & ASN1_TFLG_ADB_OID) 
		selector = OBJ_obj2nid((ASN1_OBJECT *)*sfld);
	else 
		selector = ASN1_INTEGER_get((ASN1_INTEGER *)*sfld);

	/* Try to find matching entry in table
	 * Maybe should check application types first to
	 * allow application override? Might also be useful
	 * to have a flag which indicates table is sorted and
	 * we can do a binary search. For now stick to a
	 * linear search.
	 */

	for (atbl = adb->tbl, i = 0; i < adb->tblcount; i++, atbl++)
		if (atbl->value == selector)
			return &atbl->tt;

	/* FIXME: need to search application table too */

	/* No match, return default type */
	if (!adb->default_tt)
		goto err;		
	return adb->default_tt;
	
	err:
	/* FIXME: should log the value or OID of unsupported type */
	if (nullerr)
		ASN1err(ASN1_F_ASN1_DO_ADB,
			ASN1_R_UNSUPPORTED_ANY_DEFINED_BY_TYPE);
	return NULL;
	}