
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
#include <openssl/objects.h>

int ASN1_TYPE_get(ASN1_TYPE *a)
	{
	if ((a->value.ptr != NULL) || (a->type == V_ASN1_NULL))
		return(a->type);
	else
		return(0);
	}

void ASN1_TYPE_set(ASN1_TYPE *a, int type, void *value)
	{
	if (a->value.ptr != NULL)
		{
		ASN1_TYPE **tmp_a = &a;
		ASN1_primitive_free((ASN1_VALUE **)tmp_a, NULL);
		}
	a->type=type;
	if (type == V_ASN1_BOOLEAN)
		a->value.boolean = value ? 0xff : 0;
	else
		a->value.ptr=value;
	}

int ASN1_TYPE_set1(ASN1_TYPE *a, int type, const void *value)
	{
	if (!value || (type == V_ASN1_BOOLEAN))
		{
		void *p = (void *)value;
		ASN1_TYPE_set(a, type, p);
		}
	else if (type == V_ASN1_OBJECT)
		{
		ASN1_OBJECT *odup;
		odup = OBJ_dup(value);
		if (!odup)
			return 0;
		ASN1_TYPE_set(a, type, odup);
		}
	else
		{
		ASN1_STRING *sdup;
		sdup = ASN1_STRING_dup(value);
		if (!sdup)
			return 0;
		ASN1_TYPE_set(a, type, sdup);
		}
	return 1;
	}

IMPLEMENT_STACK_OF(ASN1_TYPE)
IMPLEMENT_ASN1_SET_OF(ASN1_TYPE)

/* Returns 0 if they are equal, != 0 otherwise. */
int ASN1_TYPE_cmp(ASN1_TYPE *a, ASN1_TYPE *b)
	{
	int result = -1;

	if (!a || !b || a->type != b->type) return -1;

	switch (a->type)
		{
	case V_ASN1_OBJECT:
		result = OBJ_cmp(a->value.object, b->value.object);
		break;
	case V_ASN1_NULL:
		result = 0;	/* They do not have content. */
		break;
	case V_ASN1_INTEGER:
	case V_ASN1_NEG_INTEGER:
	case V_ASN1_ENUMERATED:
	case V_ASN1_NEG_ENUMERATED:
	case V_ASN1_BIT_STRING:
	case V_ASN1_OCTET_STRING:
	case V_ASN1_SEQUENCE:
	case V_ASN1_SET:
	case V_ASN1_NUMERICSTRING:
	case V_ASN1_PRINTABLESTRING:
	case V_ASN1_T61STRING:
	case V_ASN1_VIDEOTEXSTRING:
	case V_ASN1_IA5STRING:
	case V_ASN1_UTCTIME:
	case V_ASN1_GENERALIZEDTIME:
	case V_ASN1_GRAPHICSTRING:
	case V_ASN1_VISIBLESTRING:
	case V_ASN1_GENERALSTRING:
	case V_ASN1_UNIVERSALSTRING:
	case V_ASN1_BMPSTRING:
	case V_ASN1_UTF8STRING:
	case V_ASN1_OTHER:
	default:
		result = ASN1_STRING_cmp((ASN1_STRING *) a->value.ptr,
					 (ASN1_STRING *) b->value.ptr);
		break;
		}

	return result;
	}