
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
#include <openssl/asn1.h>
#include <openssl/objects.h>
#include <openssl/err.h>
#include <openssl/asn1t.h>
#include <string.h>

static int asn1_item_ex_combine_new(ASN1_VALUE **pval, const ASN1_ITEM *it,
								int combine);
static void asn1_item_clear(ASN1_VALUE **pval, const ASN1_ITEM *it);
static void asn1_template_clear(ASN1_VALUE **pval, const ASN1_TEMPLATE *tt);
static void asn1_primitive_clear(ASN1_VALUE **pval, const ASN1_ITEM *it);

ASN1_VALUE *ASN1_item_new(const ASN1_ITEM *it)
	{
	ASN1_VALUE *ret = NULL;
	if (ASN1_item_ex_new(&ret, it) > 0)
		return ret;
	return NULL;
	}

/* Allocate an ASN1 structure */

int ASN1_item_ex_new(ASN1_VALUE **pval, const ASN1_ITEM *it)
	{
	return asn1_item_ex_combine_new(pval, it, 0);
	}

static int asn1_item_ex_combine_new(ASN1_VALUE **pval, const ASN1_ITEM *it,
								int combine)
	{
	const ASN1_TEMPLATE *tt = NULL;
	const ASN1_COMPAT_FUNCS *cf;
	const ASN1_EXTERN_FUNCS *ef;
	const ASN1_AUX *aux = it->funcs;
	ASN1_aux_cb *asn1_cb;
	ASN1_VALUE **pseqval;
	int i;
	if (aux && aux->asn1_cb)
		asn1_cb = aux->asn1_cb;
	else
		asn1_cb = 0;

	if (!combine) *pval = NULL;

#ifdef CRYPTO_MDEBUG
	if (it->sname)
		CRYPTO_push_info(it->sname);
#endif

	switch(it->itype)
		{

		case ASN1_ITYPE_EXTERN:
		ef = it->funcs;
		if (ef && ef->asn1_ex_new)
			{
			if (!ef->asn1_ex_new(pval, it))
				goto memerr;
			}
		break;

		case ASN1_ITYPE_COMPAT:
		cf = it->funcs;
		if (cf && cf->asn1_new) {
			*pval = cf->asn1_new();
			if (!*pval)
				goto memerr;
		}
		break;

		case ASN1_ITYPE_PRIMITIVE:
		if (it->templates)
			{
			if (!ASN1_template_new(pval, it->templates))
				goto memerr;
			}
		else if (!ASN1_primitive_new(pval, it))
				goto memerr;
		break;

		case ASN1_ITYPE_MSTRING:
		if (!ASN1_primitive_new(pval, it))
				goto memerr;
		break;

		case ASN1_ITYPE_CHOICE:
		if (asn1_cb)
			{
			i = asn1_cb(ASN1_OP_NEW_PRE, pval, it, NULL);
			if (!i)
				goto auxerr;
			if (i==2)
				{
#ifdef CRYPTO_MDEBUG
				if (it->sname)
					CRYPTO_pop_info();
#endif
				return 1;
				}
			}
		if (!combine)
			{
			*pval = OPENSSL_malloc(it->size);
			if (!*pval)
				goto memerr;
			memset(*pval, 0, it->size);
			}
		asn1_set_choice_selector(pval, -1, it);
		if (asn1_cb && !asn1_cb(ASN1_OP_NEW_POST, pval, it, NULL))
				goto auxerr;
		break;

		case ASN1_ITYPE_NDEF_SEQUENCE:
		case ASN1_ITYPE_SEQUENCE:
		if (asn1_cb)
			{
			i = asn1_cb(ASN1_OP_NEW_PRE, pval, it, NULL);
			if (!i)
				goto auxerr;
			if (i==2)
				{
#ifdef CRYPTO_MDEBUG
				if (it->sname)
					CRYPTO_pop_info();
#endif
				return 1;
				}
			}
		if (!combine)
			{
			*pval = OPENSSL_malloc(it->size);
			if (!*pval)
				goto memerr;
			memset(*pval, 0, it->size);
			asn1_do_lock(pval, 0, it);
			asn1_enc_init(pval, it);
			}
		for (i = 0, tt = it->templates; i < it->tcount; tt++, i++)
			{
			pseqval = asn1_get_field_ptr(pval, tt);
			if (!ASN1_template_new(pseqval, tt))
				goto memerr;
			}
		if (asn1_cb && !asn1_cb(ASN1_OP_NEW_POST, pval, it, NULL))
				goto auxerr;
		break;
	}
#ifdef CRYPTO_MDEBUG
	if (it->sname) CRYPTO_pop_info();
#endif
	return 1;

	memerr:
	ASN1err(ASN1_F_ASN1_ITEM_EX_COMBINE_NEW, ERR_R_MALLOC_FAILURE);
#ifdef CRYPTO_MDEBUG
	if (it->sname) CRYPTO_pop_info();
#endif
	return 0;

	auxerr:
	ASN1err(ASN1_F_ASN1_ITEM_EX_COMBINE_NEW, ASN1_R_AUX_ERROR);
	ASN1_item_ex_free(pval, it);
#ifdef CRYPTO_MDEBUG
	if (it->sname) CRYPTO_pop_info();
#endif
	return 0;

	}

static void asn1_item_clear(ASN1_VALUE **pval, const ASN1_ITEM *it)
	{
	const ASN1_EXTERN_FUNCS *ef;

	switch(it->itype)
		{

		case ASN1_ITYPE_EXTERN:
		ef = it->funcs;
		if (ef && ef->asn1_ex_clear) 
			ef->asn1_ex_clear(pval, it);
		else *pval = NULL;
		break;


		case ASN1_ITYPE_PRIMITIVE:
		if (it->templates) 
			asn1_template_clear(pval, it->templates);
		else
			asn1_primitive_clear(pval, it);
		break;

		case ASN1_ITYPE_MSTRING:
		asn1_primitive_clear(pval, it);
		break;

		case ASN1_ITYPE_COMPAT:
		case ASN1_ITYPE_CHOICE:
		case ASN1_ITYPE_SEQUENCE:
		case ASN1_ITYPE_NDEF_SEQUENCE:
		*pval = NULL;
		break;
		}
	}


int ASN1_template_new(ASN1_VALUE **pval, const ASN1_TEMPLATE *tt)
	{
	const ASN1_ITEM *it = ASN1_ITEM_ptr(tt->item);
	int ret;
	if (tt->flags & ASN1_TFLG_OPTIONAL)
		{
		asn1_template_clear(pval, tt);
		return 1;
		}
	/* If ANY DEFINED BY nothing to do */

	if (tt->flags & ASN1_TFLG_ADB_MASK)
		{
		*pval = NULL;
		return 1;
		}
#ifdef CRYPTO_MDEBUG
	if (tt->field_name)
		CRYPTO_push_info(tt->field_name);
#endif
	/* If SET OF or SEQUENCE OF, its a STACK */
	if (tt->flags & ASN1_TFLG_SK_MASK)
		{
		STACK_OF(ASN1_VALUE) *skval;
		skval = sk_ASN1_VALUE_new_null();
		if (!skval)
			{
			ASN1err(ASN1_F_ASN1_TEMPLATE_NEW, ERR_R_MALLOC_FAILURE);
			ret = 0;
			goto done;
			}
		*pval = (ASN1_VALUE *)skval;
		ret = 1;
		goto done;
		}
	/* Otherwise pass it back to the item routine */
	ret = asn1_item_ex_combine_new(pval, it, tt->flags & ASN1_TFLG_COMBINE);
	done:
#ifdef CRYPTO_MDEBUG
	if (it->sname)
		CRYPTO_pop_info();
#endif
	return ret;
	}

static void asn1_template_clear(ASN1_VALUE **pval, const ASN1_TEMPLATE *tt)
	{
	/* If ADB or STACK just NULL the field */
	if (tt->flags & (ASN1_TFLG_ADB_MASK|ASN1_TFLG_SK_MASK)) 
		*pval = NULL;
	else
		asn1_item_clear(pval, ASN1_ITEM_ptr(tt->item));
	}


/* NB: could probably combine most of the real XXX_new() behaviour and junk
 * all the old functions.
 */

int ASN1_primitive_new(ASN1_VALUE **pval, const ASN1_ITEM *it)
	{
	ASN1_TYPE *typ;
	ASN1_STRING *str;
	int utype;

	if (it && it->funcs)
		{
		const ASN1_PRIMITIVE_FUNCS *pf = it->funcs;
		if (pf->prim_new)
			return pf->prim_new(pval, it);
		}

	if (!it || (it->itype == ASN1_ITYPE_MSTRING))
		utype = -1;
	else
		utype = it->utype;
	switch(utype)
		{
		case V_ASN1_OBJECT:
		*pval = (ASN1_VALUE *)OBJ_nid2obj(NID_undef);
		return 1;

		case V_ASN1_BOOLEAN:
		*(ASN1_BOOLEAN *)pval = it->size;
		return 1;

		case V_ASN1_NULL:
		*pval = (ASN1_VALUE *)1;
		return 1;

		case V_ASN1_ANY:
		typ = OPENSSL_malloc(sizeof(ASN1_TYPE));
		if (!typ)
			return 0;
		typ->value.ptr = NULL;
		typ->type = -1;
		*pval = (ASN1_VALUE *)typ;
		break;

		default:
		str = ASN1_STRING_type_new(utype);
		if (it->itype == ASN1_ITYPE_MSTRING && str)
			str->flags |= ASN1_STRING_FLAG_MSTRING;
		*pval = (ASN1_VALUE *)str;
		break;
		}
	if (*pval)
		return 1;
	return 0;
	}

static void asn1_primitive_clear(ASN1_VALUE **pval, const ASN1_ITEM *it)
	{
	int utype;
	if (it && it->funcs)
		{
		const ASN1_PRIMITIVE_FUNCS *pf = it->funcs;
		if (pf->prim_clear)
			pf->prim_clear(pval, it);
		else 
			*pval = NULL;
		return;
		}
	if (!it || (it->itype == ASN1_ITYPE_MSTRING))
		utype = -1;
	else
		utype = it->utype;
	if (utype == V_ASN1_BOOLEAN)
		*(ASN1_BOOLEAN *)pval = it->size;
	else *pval = NULL;
	}