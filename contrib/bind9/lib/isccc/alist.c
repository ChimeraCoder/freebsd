
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

/* $Id: alist.c,v 1.8 2007/08/28 07:20:43 tbox Exp $ */

/*! \file */

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include <isccc/alist.h>
#include <isc/assertions.h>
#include <isccc/result.h>
#include <isccc/sexpr.h>
#include <isccc/util.h>

#define CAR(s)			(s)->value.as_dottedpair.car
#define CDR(s)			(s)->value.as_dottedpair.cdr

#define ALIST_TAG		"*alist*"
#define MAX_INDENT		64

static char spaces[MAX_INDENT + 1] = 
	"                                                                ";

isccc_sexpr_t *
isccc_alist_create(void)
{
	isccc_sexpr_t *alist, *tag;

	tag = isccc_sexpr_fromstring(ALIST_TAG);
	if (tag == NULL)
		return (NULL);
	alist = isccc_sexpr_cons(tag, NULL);
	if (alist == NULL) {
		isccc_sexpr_free(&tag);
		return (NULL);
	}

	return (alist);
}

isc_boolean_t
isccc_alist_alistp(isccc_sexpr_t *alist)
{
	isccc_sexpr_t *car;

	if (alist == NULL || alist->type != ISCCC_SEXPRTYPE_DOTTEDPAIR)
		return (ISC_FALSE);
	car = CAR(alist);
	if (car == NULL || car->type != ISCCC_SEXPRTYPE_STRING)
		return (ISC_FALSE);
	if (strcmp(car->value.as_string, ALIST_TAG) != 0)
		return (ISC_FALSE);
	return (ISC_TRUE);
}

isc_boolean_t
isccc_alist_emptyp(isccc_sexpr_t *alist)
{
	REQUIRE(isccc_alist_alistp(alist));

	if (CDR(alist) == NULL)
		return (ISC_TRUE);
	return (ISC_FALSE);
}

isccc_sexpr_t *
isccc_alist_first(isccc_sexpr_t *alist)
{
	REQUIRE(isccc_alist_alistp(alist));

	return (CDR(alist));
}

isccc_sexpr_t *
isccc_alist_assq(isccc_sexpr_t *alist, const char *key)
{
	isccc_sexpr_t *car, *caar;

	REQUIRE(isccc_alist_alistp(alist));

	/*
	 * Skip alist type tag.
	 */
	alist = CDR(alist);

	while (alist != NULL) {
		INSIST(alist->type == ISCCC_SEXPRTYPE_DOTTEDPAIR);
		car = CAR(alist);
		INSIST(car->type == ISCCC_SEXPRTYPE_DOTTEDPAIR);
		caar = CAR(car);
		if (caar->type == ISCCC_SEXPRTYPE_STRING &&
		    strcmp(caar->value.as_string, key) == 0)
			return (car);
		alist = CDR(alist);
	}

	return (NULL);
}

void
isccc_alist_delete(isccc_sexpr_t *alist, const char *key)
{
	isccc_sexpr_t *car, *caar, *rest, *prev;

	REQUIRE(isccc_alist_alistp(alist));

	prev = alist;
	rest = CDR(alist);
	while (rest != NULL) {
		INSIST(rest->type == ISCCC_SEXPRTYPE_DOTTEDPAIR);
		car = CAR(rest);
		INSIST(car != NULL && car->type == ISCCC_SEXPRTYPE_DOTTEDPAIR);
		caar = CAR(car);
		if (caar->type == ISCCC_SEXPRTYPE_STRING &&
		    strcmp(caar->value.as_string, key) == 0) {
			CDR(prev) = CDR(rest);
			CDR(rest) = NULL;
			isccc_sexpr_free(&rest);
			break;
		}
		prev = rest;
		rest = CDR(rest);
	}
}

isccc_sexpr_t *
isccc_alist_define(isccc_sexpr_t *alist, const char *key, isccc_sexpr_t *value)
{
	isccc_sexpr_t *kv, *k, *elt;

	kv = isccc_alist_assq(alist, key);
	if (kv == NULL) {
		/*
		 * New association.
		 */
		k = isccc_sexpr_fromstring(key);
		if (k == NULL)
			return (NULL);
		kv = isccc_sexpr_cons(k, value);
		if (kv == NULL) {
			isccc_sexpr_free(&kv);
			return (NULL);
		}
		elt = isccc_sexpr_addtolist(&alist, kv);
		if (elt == NULL) {
			isccc_sexpr_free(&kv);
			return (NULL);
		}
	} else {
		/*
		 * We've already got an entry for this key.  Replace it.
		 */
		isccc_sexpr_free(&CDR(kv));
		CDR(kv) = value;
	}

	return (kv);
}

isccc_sexpr_t *
isccc_alist_definestring(isccc_sexpr_t *alist, const char *key, const char *str)
{
	isccc_sexpr_t *v, *kv;

	v = isccc_sexpr_fromstring(str);
	if (v == NULL)
		return (NULL);
	kv = isccc_alist_define(alist, key, v);
	if (kv == NULL)
		isccc_sexpr_free(&v);

	return (kv);
}

isccc_sexpr_t *
isccc_alist_definebinary(isccc_sexpr_t *alist, const char *key, isccc_region_t *r)
{
	isccc_sexpr_t *v, *kv;

	v = isccc_sexpr_frombinary(r);
	if (v == NULL)
		return (NULL);
	kv = isccc_alist_define(alist, key, v);
	if (kv == NULL)
		isccc_sexpr_free(&v);

	return (kv);
}

isccc_sexpr_t *
isccc_alist_lookup(isccc_sexpr_t *alist, const char *key)
{
	isccc_sexpr_t *kv;

	kv = isccc_alist_assq(alist, key);
	if (kv != NULL)
		return (CDR(kv));
	return (NULL);
}

isc_result_t
isccc_alist_lookupstring(isccc_sexpr_t *alist, const char *key, char **strp)
{
	isccc_sexpr_t *kv, *v;

	kv = isccc_alist_assq(alist, key);
	if (kv != NULL) {
		v = CDR(kv);
		if (isccc_sexpr_stringp(v)) {
			if (strp != NULL)
				*strp = isccc_sexpr_tostring(v);
			return (ISC_R_SUCCESS);
		} else
			return (ISC_R_EXISTS);
	}

	return (ISC_R_NOTFOUND);
}

isc_result_t
isccc_alist_lookupbinary(isccc_sexpr_t *alist, const char *key, isccc_region_t **r)
{
	isccc_sexpr_t *kv, *v;

	kv = isccc_alist_assq(alist, key);
	if (kv != NULL) {
		v = CDR(kv);
		if (isccc_sexpr_binaryp(v)) {
			if (r != NULL)
				*r = isccc_sexpr_tobinary(v);
			return (ISC_R_SUCCESS);
		} else
			return (ISC_R_EXISTS);
	}

	return (ISC_R_NOTFOUND);
}

void
isccc_alist_prettyprint(isccc_sexpr_t *sexpr, unsigned int indent, FILE *stream)
{
	isccc_sexpr_t *elt, *kv, *k, *v;

	if (isccc_alist_alistp(sexpr)) {
		fprintf(stream, "{\n");
		indent += 4;
		for (elt = isccc_alist_first(sexpr);
		     elt != NULL;
		     elt = CDR(elt)) {
			kv = CAR(elt);
			INSIST(isccc_sexpr_listp(kv));
			k = CAR(kv);
			v = CDR(kv);
			INSIST(isccc_sexpr_stringp(k));
			fprintf(stream, "%.*s%s => ", (int)indent, spaces,
				isccc_sexpr_tostring(k));
			isccc_alist_prettyprint(v, indent, stream);
			if (CDR(elt) != NULL)
				fprintf(stream, ",");
			fprintf(stream, "\n");
		}
		indent -= 4;
		fprintf(stream, "%.*s}", (int)indent, spaces);
	} else if (isccc_sexpr_listp(sexpr)) {
		fprintf(stream, "(\n");
		indent += 4;
		for (elt = sexpr;
		     elt != NULL;
		     elt = CDR(elt)) {
			fprintf(stream, "%.*s", (int)indent, spaces);
			isccc_alist_prettyprint(CAR(elt), indent, stream);
			if (CDR(elt) != NULL)
				fprintf(stream, ",");
			fprintf(stream, "\n");
		}
		indent -= 4;
		fprintf(stream, "%.*s)", (int)indent, spaces);
	} else
		isccc_sexpr_print(sexpr, stream);
}