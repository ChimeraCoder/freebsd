
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

/* Part of the code in here was originally in conf.c, which is now removed */

#ifndef CONF_DEBUG
# undef NDEBUG /* avoid conflicting definitions */
# define NDEBUG
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/conf.h>
#include <openssl/conf_api.h>
#include "e_os.h"

static void value_free_hash_doall_arg(CONF_VALUE *a,
				      LHASH_OF(CONF_VALUE) *conf);
static void value_free_stack_doall(CONF_VALUE *a);
static IMPLEMENT_LHASH_DOALL_ARG_FN(value_free_hash, CONF_VALUE,
				    LHASH_OF(CONF_VALUE))
static IMPLEMENT_LHASH_DOALL_FN(value_free_stack, CONF_VALUE)

/* Up until OpenSSL 0.9.5a, this was get_section */
CONF_VALUE *_CONF_get_section(const CONF *conf, const char *section)
	{
	CONF_VALUE *v,vv;

	if ((conf == NULL) || (section == NULL)) return(NULL);
	vv.name=NULL;
	vv.section=(char *)section;
	v=lh_CONF_VALUE_retrieve(conf->data,&vv);
	return(v);
	}

/* Up until OpenSSL 0.9.5a, this was CONF_get_section */
STACK_OF(CONF_VALUE) *_CONF_get_section_values(const CONF *conf,
					       const char *section)
	{
	CONF_VALUE *v;

	v=_CONF_get_section(conf,section);
	if (v != NULL)
		return((STACK_OF(CONF_VALUE) *)v->value);
	else
		return(NULL);
	}

int _CONF_add_string(CONF *conf, CONF_VALUE *section, CONF_VALUE *value)
	{
	CONF_VALUE *v = NULL;
	STACK_OF(CONF_VALUE) *ts;

	ts = (STACK_OF(CONF_VALUE) *)section->value;

	value->section=section->section;	
	if (!sk_CONF_VALUE_push(ts,value))
		{
		return 0;
		}

	v = lh_CONF_VALUE_insert(conf->data, value);
	if (v != NULL)
		{
		(void)sk_CONF_VALUE_delete_ptr(ts,v);
		OPENSSL_free(v->name);
		OPENSSL_free(v->value);
		OPENSSL_free(v);
		}
	return 1;
	}

char *_CONF_get_string(const CONF *conf, const char *section, const char *name)
	{
	CONF_VALUE *v,vv;
	char *p;

	if (name == NULL) return(NULL);
	if (conf != NULL)
		{
		if (section != NULL)
			{
			vv.name=(char *)name;
			vv.section=(char *)section;
			v=lh_CONF_VALUE_retrieve(conf->data,&vv);
			if (v != NULL) return(v->value);
			if (strcmp(section,"ENV") == 0)
				{
				p=getenv(name);
				if (p != NULL) return(p);
				}
			}
		vv.section="default";
		vv.name=(char *)name;
		v=lh_CONF_VALUE_retrieve(conf->data,&vv);
		if (v != NULL)
			return(v->value);
		else
			return(NULL);
		}
	else
		return(getenv(name));
	}

#if 0 /* There's no way to provide error checking with this function, so
	 force implementors of the higher levels to get a string and read
	 the number themselves. */
long _CONF_get_number(CONF *conf, char *section, char *name)
	{
	char *str;
	long ret=0;

	str=_CONF_get_string(conf,section,name);
	if (str == NULL) return(0);
	for (;;)
		{
		if (conf->meth->is_number(conf, *str))
			ret=ret*10+conf->meth->to_int(conf, *str);
		else
			return(ret);
		str++;
		}
	}
#endif

static unsigned long conf_value_hash(const CONF_VALUE *v)
	{
	return (lh_strhash(v->section)<<2)^lh_strhash(v->name);
	}
static IMPLEMENT_LHASH_HASH_FN(conf_value, CONF_VALUE)

static int conf_value_cmp(const CONF_VALUE *a, const CONF_VALUE *b)
	{
	int i;

	if (a->section != b->section)
		{
		i=strcmp(a->section,b->section);
		if (i) return(i);
		}

	if ((a->name != NULL) && (b->name != NULL))
		{
		i=strcmp(a->name,b->name);
		return(i);
		}
	else if (a->name == b->name)
		return(0);
	else
		return((a->name == NULL)?-1:1);
	}
static IMPLEMENT_LHASH_COMP_FN(conf_value, CONF_VALUE)

int _CONF_new_data(CONF *conf)
	{
	if (conf == NULL)
		{
		return 0;
		}
	if (conf->data == NULL)
		if ((conf->data = lh_CONF_VALUE_new()) == NULL)
			{
			return 0;
			}
	return 1;
	}

void _CONF_free_data(CONF *conf)
	{
	if (conf == NULL || conf->data == NULL) return;

	lh_CONF_VALUE_down_load(conf->data)=0; /* evil thing to make
				  * sure the 'OPENSSL_free()' works as
				  * expected */
	lh_CONF_VALUE_doall_arg(conf->data,
				LHASH_DOALL_ARG_FN(value_free_hash),
				LHASH_OF(CONF_VALUE), conf->data);

	/* We now have only 'section' entries in the hash table.
	 * Due to problems with */

	lh_CONF_VALUE_doall(conf->data, LHASH_DOALL_FN(value_free_stack));
	lh_CONF_VALUE_free(conf->data);
	}

static void value_free_hash_doall_arg(CONF_VALUE *a, LHASH_OF(CONF_VALUE) *conf)
	{
	if (a->name != NULL)
		(void)lh_CONF_VALUE_delete(conf,a);
	}

static void value_free_stack_doall(CONF_VALUE *a)
	{
	CONF_VALUE *vv;
	STACK_OF(CONF_VALUE) *sk;
	int i;

	if (a->name != NULL) return;

	sk=(STACK_OF(CONF_VALUE) *)a->value;
	for (i=sk_CONF_VALUE_num(sk)-1; i>=0; i--)
		{
		vv=sk_CONF_VALUE_value(sk,i);
		OPENSSL_free(vv->value);
		OPENSSL_free(vv->name);
		OPENSSL_free(vv);
		}
	if (sk != NULL) sk_CONF_VALUE_free(sk);
	OPENSSL_free(a->section);
	OPENSSL_free(a);
	}

/* Up until OpenSSL 0.9.5a, this was new_section */
CONF_VALUE *_CONF_new_section(CONF *conf, const char *section)
	{
	STACK_OF(CONF_VALUE) *sk=NULL;
	int ok=0,i;
	CONF_VALUE *v=NULL,*vv;

	if ((sk=sk_CONF_VALUE_new_null()) == NULL)
		goto err;
	if ((v=OPENSSL_malloc(sizeof(CONF_VALUE))) == NULL)
		goto err;
	i=strlen(section)+1;
	if ((v->section=OPENSSL_malloc(i)) == NULL)
		goto err;

	memcpy(v->section,section,i);
	v->name=NULL;
	v->value=(char *)sk;
	
	vv=lh_CONF_VALUE_insert(conf->data,v);
	OPENSSL_assert(vv == NULL);
	ok=1;
err:
	if (!ok)
		{
		if (sk != NULL) sk_CONF_VALUE_free(sk);
		if (v != NULL) OPENSSL_free(v);
		v=NULL;
		}
	return(v);
	}

IMPLEMENT_STACK_OF(CONF_VALUE)