
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

/* Code for stacks
 * Author - Eric Young v 1.0
 * 1.2 eay 12-Mar-97 -	Modified sk_find so that it _DOES_ return the
 *			lowest index for the searched item.
 *
 * 1.1 eay - Take from netdb and added to SSLeay
 *
 * 1.0 eay - First version 29/07/92
 */
#include <stdio.h>
#include "cryptlib.h"
#include <openssl/stack.h>
#include <openssl/objects.h>

#undef MIN_NODES
#define MIN_NODES	4

const char STACK_version[]="Stack" OPENSSL_VERSION_PTEXT;

#include <errno.h>

int (*sk_set_cmp_func(_STACK *sk, int (*c)(const void *, const void *)))
		(const void *, const void *)
	{
	int (*old)(const void *,const void *)=sk->comp;

	if (sk->comp != c)
		sk->sorted=0;
	sk->comp=c;

	return old;
	}

_STACK *sk_dup(_STACK *sk)
	{
	_STACK *ret;
	char **s;

	if ((ret=sk_new(sk->comp)) == NULL) goto err;
	s=(char **)OPENSSL_realloc((char *)ret->data,
		(unsigned int)sizeof(char *)*sk->num_alloc);
	if (s == NULL) goto err;
	ret->data=s;

	ret->num=sk->num;
	memcpy(ret->data,sk->data,sizeof(char *)*sk->num);
	ret->sorted=sk->sorted;
	ret->num_alloc=sk->num_alloc;
	ret->comp=sk->comp;
	return(ret);
err:
	if(ret)
		sk_free(ret);
	return(NULL);
	}

_STACK *sk_new_null(void)
	{
	return sk_new((int (*)(const void *, const void *))0);
	}

_STACK *sk_new(int (*c)(const void *, const void *))
	{
	_STACK *ret;
	int i;

	if ((ret=OPENSSL_malloc(sizeof(_STACK))) == NULL)
		goto err;
	if ((ret->data=OPENSSL_malloc(sizeof(char *)*MIN_NODES)) == NULL)
		goto err;
	for (i=0; i<MIN_NODES; i++)
		ret->data[i]=NULL;
	ret->comp=c;
	ret->num_alloc=MIN_NODES;
	ret->num=0;
	ret->sorted=0;
	return(ret);
err:
	if(ret)
		OPENSSL_free(ret);
	return(NULL);
	}

int sk_insert(_STACK *st, void *data, int loc)
	{
	char **s;

	if(st == NULL) return 0;
	if (st->num_alloc <= st->num+1)
		{
		s=OPENSSL_realloc((char *)st->data,
			(unsigned int)sizeof(char *)*st->num_alloc*2);
		if (s == NULL)
			return(0);
		st->data=s;
		st->num_alloc*=2;
		}
	if ((loc >= (int)st->num) || (loc < 0))
		st->data[st->num]=data;
	else
		{
		int i;
		char **f,**t;

		f=st->data;
		t=&(st->data[1]);
		for (i=st->num; i>=loc; i--)
			t[i]=f[i];
			
#ifdef undef /* no memmove on sunos :-( */
		memmove(&(st->data[loc+1]),
			&(st->data[loc]),
			sizeof(char *)*(st->num-loc));
#endif
		st->data[loc]=data;
		}
	st->num++;
	st->sorted=0;
	return(st->num);
	}

void *sk_delete_ptr(_STACK *st, void *p)
	{
	int i;

	for (i=0; i<st->num; i++)
		if (st->data[i] == p)
			return(sk_delete(st,i));
	return(NULL);
	}

void *sk_delete(_STACK *st, int loc)
	{
	char *ret;
	int i,j;

	if(!st || (loc < 0) || (loc >= st->num)) return NULL;

	ret=st->data[loc];
	if (loc != st->num-1)
		{
		j=st->num-1;
		for (i=loc; i<j; i++)
			st->data[i]=st->data[i+1];
		/* In theory memcpy is not safe for this
		 * memcpy( &(st->data[loc]),
		 *	&(st->data[loc+1]),
		 *	sizeof(char *)*(st->num-loc-1));
		 */
		}
	st->num--;
	return(ret);
	}

static int internal_find(_STACK *st, void *data, int ret_val_options)
	{
	const void * const *r;
	int i;

	if(st == NULL) return -1;

	if (st->comp == NULL)
		{
		for (i=0; i<st->num; i++)
			if (st->data[i] == data)
				return(i);
		return(-1);
		}
	sk_sort(st);
	if (data == NULL) return(-1);
	r=OBJ_bsearch_ex_(&data,st->data,st->num,sizeof(void *),st->comp,
			  ret_val_options);
	if (r == NULL) return(-1);
	return (int)((char **)r-st->data);
	}

int sk_find(_STACK *st, void *data)
	{
	return internal_find(st, data, OBJ_BSEARCH_FIRST_VALUE_ON_MATCH);
	}
int sk_find_ex(_STACK *st, void *data)
	{
	return internal_find(st, data, OBJ_BSEARCH_VALUE_ON_NOMATCH);
	}

int sk_push(_STACK *st, void *data)
	{
	return(sk_insert(st,data,st->num));
	}

int sk_unshift(_STACK *st, void *data)
	{
	return(sk_insert(st,data,0));
	}

void *sk_shift(_STACK *st)
	{
	if (st == NULL) return(NULL);
	if (st->num <= 0) return(NULL);
	return(sk_delete(st,0));
	}

void *sk_pop(_STACK *st)
	{
	if (st == NULL) return(NULL);
	if (st->num <= 0) return(NULL);
	return(sk_delete(st,st->num-1));
	}

void sk_zero(_STACK *st)
	{
	if (st == NULL) return;
	if (st->num <= 0) return;
	memset((char *)st->data,0,sizeof(st->data)*st->num);
	st->num=0;
	}

void sk_pop_free(_STACK *st, void (*func)(void *))
	{
	int i;

	if (st == NULL) return;
	for (i=0; i<st->num; i++)
		if (st->data[i] != NULL)
			func(st->data[i]);
	sk_free(st);
	}

void sk_free(_STACK *st)
	{
	if (st == NULL) return;
	if (st->data != NULL) OPENSSL_free(st->data);
	OPENSSL_free(st);
	}

int sk_num(const _STACK *st)
{
	if(st == NULL) return -1;
	return st->num;
}

void *sk_value(const _STACK *st, int i)
{
	if(!st || (i < 0) || (i >= st->num)) return NULL;
	return st->data[i];
}

void *sk_set(_STACK *st, int i, void *value)
{
	if(!st || (i < 0) || (i >= st->num)) return NULL;
	return (st->data[i] = value);
}

void sk_sort(_STACK *st)
	{
	if (st && !st->sorted)
		{
		int (*comp_func)(const void *,const void *);

		/* same comment as in sk_find ... previously st->comp was declared
		 * as a (void*,void*) callback type, but this made the population
		 * of the callback pointer illogical - our callbacks compare
		 * type** with type**, so we leave the casting until absolutely
		 * necessary (ie. "now"). */
		comp_func=(int (*)(const void *,const void *))(st->comp);
		qsort(st->data,st->num,sizeof(char *), comp_func);
		st->sorted=1;
		}
	}

int sk_is_sorted(const _STACK *st)
	{
	if (!st)
		return 1;
	return st->sorted;
	}