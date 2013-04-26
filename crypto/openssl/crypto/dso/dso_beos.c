
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
#include <string.h>
#include "cryptlib.h"
#include <openssl/dso.h>

#if !defined(OPENSSL_SYS_BEOS)
DSO_METHOD *DSO_METHOD_beos(void)
	{
	return NULL;
	}
#else

#include <kernel/image.h>

static int beos_load(DSO *dso);
static int beos_unload(DSO *dso);
static void *beos_bind_var(DSO *dso, const char *symname);
static DSO_FUNC_TYPE beos_bind_func(DSO *dso, const char *symname);
#if 0
static int beos_unbind_var(DSO *dso, char *symname, void *symptr);
static int beos_unbind_func(DSO *dso, char *symname, DSO_FUNC_TYPE symptr);
static int beos_init(DSO *dso);
static int beos_finish(DSO *dso);
static long beos_ctrl(DSO *dso, int cmd, long larg, void *parg);
#endif
static char *beos_name_converter(DSO *dso, const char *filename);

static DSO_METHOD dso_meth_beos = {
	"OpenSSL 'beos' shared library method",
	beos_load,
	beos_unload,
	beos_bind_var,
	beos_bind_func,
/* For now, "unbind" doesn't exist */
#if 0
	NULL, /* unbind_var */
	NULL, /* unbind_func */
#endif
	NULL, /* ctrl */
	beos_name_converter,
	NULL, /* init */
	NULL  /* finish */
	};

DSO_METHOD *DSO_METHOD_beos(void)
	{
	return(&dso_meth_beos);
	}

/* For this DSO_METHOD, our meth_data STACK will contain;
 * (i) a pointer to the handle (image_id) returned from
 *     load_add_on().
 */

static int beos_load(DSO *dso)
	{
	image_id id;
	/* See applicable comments from dso_dl.c */
	char *filename = DSO_convert_filename(dso, NULL);

	if(filename == NULL)
		{
		DSOerr(DSO_F_BEOS_LOAD,DSO_R_NO_FILENAME);
		goto err;
		}
	id = load_add_on(filename);
	if(id < 1)
		{
		DSOerr(DSO_F_BEOS_LOAD,DSO_R_LOAD_FAILED);
		ERR_add_error_data(3, "filename(", filename, ")");
		goto err;
		}
	if(!sk_push(dso->meth_data, (char *)id))
		{
		DSOerr(DSO_F_BEOS_LOAD,DSO_R_STACK_ERROR);
		goto err;
		}
	/* Success */
	dso->loaded_filename = filename;
	return(1);
err:
	/* Cleanup !*/
	if(filename != NULL)
		OPENSSL_free(filename);
	if(id > 0)
		unload_add_on(id);
	return(0);
	}

static int beos_unload(DSO *dso)
	{
	image_id id;
	if(dso == NULL)
		{
		DSOerr(DSO_F_BEOS_UNLOAD,ERR_R_PASSED_NULL_PARAMETER);
		return(0);
		}
	if(sk_num(dso->meth_data) < 1)
		return(1);
	id = (image_id)sk_pop(dso->meth_data);
	if(id < 1)
		{
		DSOerr(DSO_F_BEOS_UNLOAD,DSO_R_NULL_HANDLE);
		return(0);
		}
	if(unload_add_on(id) != B_OK)
		{
		DSOerr(DSO_F_BEOS_UNLOAD,DSO_R_UNLOAD_FAILED);
		/* We should push the value back onto the stack in
		 * case of a retry. */
		sk_push(dso->meth_data, (char *)id);
		return(0);
		}
	return(1);
	}

static void *beos_bind_var(DSO *dso, const char *symname)
	{
	image_id id;
	void *sym;

	if((dso == NULL) || (symname == NULL))
		{
		DSOerr(DSO_F_BEOS_BIND_VAR,ERR_R_PASSED_NULL_PARAMETER);
		return(NULL);
		}
	if(sk_num(dso->meth_data) < 1)
		{
		DSOerr(DSO_F_BEOS_BIND_VAR,DSO_R_STACK_ERROR);
		return(NULL);
		}
	id = (image_id)sk_value(dso->meth_data, sk_num(dso->meth_data) - 1);
	if(id < 1)
		{
		DSOerr(DSO_F_BEOS_BIND_VAR,DSO_R_NULL_HANDLE);
		return(NULL);
		}
	if(get_image_symbol(id, symname, B_SYMBOL_TYPE_DATA, &sym) != B_OK)
		{
		DSOerr(DSO_F_BEOS_BIND_VAR,DSO_R_SYM_FAILURE);
		ERR_add_error_data(3, "symname(", symname, ")");
		return(NULL);
		}
	return(sym);
	}

static DSO_FUNC_TYPE beos_bind_func(DSO *dso, const char *symname)
	{
	image_id id;
	void *sym;

	if((dso == NULL) || (symname == NULL))
		{
		DSOerr(DSO_F_BEOS_BIND_FUNC,ERR_R_PASSED_NULL_PARAMETER);
		return(NULL);
		}
	if(sk_num(dso->meth_data) < 1)
		{
		DSOerr(DSO_F_BEOS_BIND_FUNC,DSO_R_STACK_ERROR);
		return(NULL);
		}
	id = (image_id)sk_value(dso->meth_data, sk_num(dso->meth_data) - 1);
	if(id < 1)
		{
		DSOerr(DSO_F_BEOS_BIND_FUNC,DSO_R_NULL_HANDLE);
		return(NULL);
		}
	if(get_image_symbol(id, symname, B_SYMBOL_TYPE_TEXT, &sym) != B_OK)
		{
		DSOerr(DSO_F_BEOS_BIND_FUNC,DSO_R_SYM_FAILURE);
		ERR_add_error_data(3, "symname(", symname, ")");
		return(NULL);
		}
	return((DSO_FUNC_TYPE)sym);
	}

/* This one is the same as the one in dlfcn */
static char *beos_name_converter(DSO *dso, const char *filename)
	{
	char *translated;
	int len, rsize, transform;

	len = strlen(filename);
	rsize = len + 1;
	transform = (strstr(filename, "/") == NULL);
	if(transform)
		{
		/* We will convert this to "%s.so" or "lib%s.so" */
		rsize += 3;	/* The length of ".so" */
		if ((DSO_flags(dso) & DSO_FLAG_NAME_TRANSLATION_EXT_ONLY) == 0)
			rsize += 3; /* The length of "lib" */
		}
	translated = OPENSSL_malloc(rsize);
	if(translated == NULL)
		{
		DSOerr(DSO_F_BEOS_NAME_CONVERTER,
				DSO_R_NAME_TRANSLATION_FAILED);
		return(NULL);
		}
	if(transform)
		{
		if ((DSO_flags(dso) & DSO_FLAG_NAME_TRANSLATION_EXT_ONLY) == 0)
			sprintf(translated, "lib%s.so", filename);
		else
			sprintf(translated, "%s.so", filename);
		}
	else
		sprintf(translated, "%s", filename);
	return(translated);
	}

#endif