
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

/* NOTE: this file was auto generated by the mkerr.pl script: any changes
 * made to it will be overwritten when the script next updates this file,
 * only reason strings will be preserved.
 */

#include <stdio.h>
#include <openssl/err.h>
#include "e_capi_err.h"

/* BEGIN ERROR CODES */
#ifndef OPENSSL_NO_ERR

#define ERR_FUNC(func) ERR_PACK(0,func,0)
#define ERR_REASON(reason) ERR_PACK(0,0,reason)

static ERR_STRING_DATA CAPI_str_functs[]=
	{
{ERR_FUNC(CAPI_F_CAPI_CERT_GET_FNAME),	"CAPI_CERT_GET_FNAME"},
{ERR_FUNC(CAPI_F_CAPI_CTRL),	"CAPI_CTRL"},
{ERR_FUNC(CAPI_F_CAPI_CTX_NEW),	"CAPI_CTX_NEW"},
{ERR_FUNC(CAPI_F_CAPI_CTX_SET_PROVNAME),	"CAPI_CTX_SET_PROVNAME"},
{ERR_FUNC(CAPI_F_CAPI_DSA_DO_SIGN),	"CAPI_DSA_DO_SIGN"},
{ERR_FUNC(CAPI_F_CAPI_GET_KEY),	"CAPI_GET_KEY"},
{ERR_FUNC(CAPI_F_CAPI_GET_PKEY),	"CAPI_GET_PKEY"},
{ERR_FUNC(CAPI_F_CAPI_GET_PROVNAME),	"CAPI_GET_PROVNAME"},
{ERR_FUNC(CAPI_F_CAPI_GET_PROV_INFO),	"CAPI_GET_PROV_INFO"},
{ERR_FUNC(CAPI_F_CAPI_INIT),	"CAPI_INIT"},
{ERR_FUNC(CAPI_F_CAPI_LIST_CONTAINERS),	"CAPI_LIST_CONTAINERS"},
{ERR_FUNC(CAPI_F_CAPI_LOAD_PRIVKEY),	"CAPI_LOAD_PRIVKEY"},
{ERR_FUNC(CAPI_F_CAPI_OPEN_STORE),	"CAPI_OPEN_STORE"},
{ERR_FUNC(CAPI_F_CAPI_RSA_PRIV_DEC),	"CAPI_RSA_PRIV_DEC"},
{ERR_FUNC(CAPI_F_CAPI_RSA_PRIV_ENC),	"CAPI_RSA_PRIV_ENC"},
{ERR_FUNC(CAPI_F_CAPI_RSA_SIGN),	"CAPI_RSA_SIGN"},
{ERR_FUNC(CAPI_F_CERT_SELECT_DIALOG),	"CERT_SELECT_DIALOG"},
{ERR_FUNC(CAPI_F_CLIENT_CERT_SELECT),	"CLIENT_CERT_SELECT"},
{ERR_FUNC(CAPI_F_WIDE_TO_ASC),	"WIDE_TO_ASC"},
{0,NULL}
	};

static ERR_STRING_DATA CAPI_str_reasons[]=
	{
{ERR_REASON(CAPI_R_CANT_CREATE_HASH_OBJECT),"cant create hash object"},
{ERR_REASON(CAPI_R_CANT_FIND_CAPI_CONTEXT),"cant find capi context"},
{ERR_REASON(CAPI_R_CANT_GET_KEY)         ,"cant get key"},
{ERR_REASON(CAPI_R_CANT_SET_HASH_VALUE)  ,"cant set hash value"},
{ERR_REASON(CAPI_R_CRYPTACQUIRECONTEXT_ERROR),"cryptacquirecontext error"},
{ERR_REASON(CAPI_R_CRYPTENUMPROVIDERS_ERROR),"cryptenumproviders error"},
{ERR_REASON(CAPI_R_DECRYPT_ERROR)        ,"decrypt error"},
{ERR_REASON(CAPI_R_ENGINE_NOT_INITIALIZED),"engine not initialized"},
{ERR_REASON(CAPI_R_ENUMCONTAINERS_ERROR) ,"enumcontainers error"},
{ERR_REASON(CAPI_R_ERROR_ADDING_CERT)    ,"error adding cert"},
{ERR_REASON(CAPI_R_ERROR_CREATING_STORE) ,"error creating store"},
{ERR_REASON(CAPI_R_ERROR_GETTING_FRIENDLY_NAME),"error getting friendly name"},
{ERR_REASON(CAPI_R_ERROR_GETTING_KEY_PROVIDER_INFO),"error getting key provider info"},
{ERR_REASON(CAPI_R_ERROR_OPENING_STORE)  ,"error opening store"},
{ERR_REASON(CAPI_R_ERROR_SIGNING_HASH)   ,"error signing hash"},
{ERR_REASON(CAPI_R_FUNCTION_NOT_SUPPORTED),"function not supported"},
{ERR_REASON(CAPI_R_GETUSERKEY_ERROR)     ,"getuserkey error"},
{ERR_REASON(CAPI_R_INVALID_DIGEST_LENGTH),"invalid digest length"},
{ERR_REASON(CAPI_R_INVALID_DSA_PUBLIC_KEY_BLOB_MAGIC_NUMBER),"invalid dsa public key blob magic number"},
{ERR_REASON(CAPI_R_INVALID_LOOKUP_METHOD),"invalid lookup method"},
{ERR_REASON(CAPI_R_INVALID_PUBLIC_KEY_BLOB),"invalid public key blob"},
{ERR_REASON(CAPI_R_INVALID_RSA_PUBLIC_KEY_BLOB_MAGIC_NUMBER),"invalid rsa public key blob magic number"},
{ERR_REASON(CAPI_R_PUBKEY_EXPORT_ERROR)  ,"pubkey export error"},
{ERR_REASON(CAPI_R_PUBKEY_EXPORT_LENGTH_ERROR),"pubkey export length error"},
{ERR_REASON(CAPI_R_UNKNOWN_COMMAND)      ,"unknown command"},
{ERR_REASON(CAPI_R_UNSUPPORTED_ALGORITHM_NID),"unsupported algorithm nid"},
{ERR_REASON(CAPI_R_UNSUPPORTED_PADDING)  ,"unsupported padding"},
{ERR_REASON(CAPI_R_UNSUPPORTED_PUBLIC_KEY_ALGORITHM),"unsupported public key algorithm"},
{ERR_REASON(CAPI_R_WIN32_ERROR)          ,"win32 error"},
{0,NULL}
	};

#endif

#ifdef CAPI_LIB_NAME
static ERR_STRING_DATA CAPI_lib_name[]=
        {
{0	,CAPI_LIB_NAME},
{0,NULL}
	};
#endif


static int CAPI_lib_error_code=0;
static int CAPI_error_init=1;

static void ERR_load_CAPI_strings(void)
	{
	if (CAPI_lib_error_code == 0)
		CAPI_lib_error_code=ERR_get_next_error_library();

	if (CAPI_error_init)
		{
		CAPI_error_init=0;
#ifndef OPENSSL_NO_ERR
		ERR_load_strings(CAPI_lib_error_code,CAPI_str_functs);
		ERR_load_strings(CAPI_lib_error_code,CAPI_str_reasons);
#endif

#ifdef CAPI_LIB_NAME
		CAPI_lib_name->error = ERR_PACK(CAPI_lib_error_code,0,0);
		ERR_load_strings(0,CAPI_lib_name);
#endif
		}
	}

static void ERR_unload_CAPI_strings(void)
	{
	if (CAPI_error_init == 0)
		{
#ifndef OPENSSL_NO_ERR
		ERR_unload_strings(CAPI_lib_error_code,CAPI_str_functs);
		ERR_unload_strings(CAPI_lib_error_code,CAPI_str_reasons);
#endif

#ifdef CAPI_LIB_NAME
		ERR_unload_strings(0,CAPI_lib_name);
#endif
		CAPI_error_init=1;
		}
	}

static void ERR_CAPI_error(int function, int reason, char *file, int line)
	{
	if (CAPI_lib_error_code == 0)
		CAPI_lib_error_code=ERR_get_next_error_library();
	ERR_PUT_error(CAPI_lib_error_code,function,reason,file,line);
	}