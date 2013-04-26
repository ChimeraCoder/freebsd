
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
#include <ctype.h>
#include <openssl/crypto.h>
#include "cryptlib.h"
#include <openssl/conf.h>
#include <openssl/dso.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#ifdef OPENSSL_FIPS
#include <openssl/fips.h>
#endif


/* Algorithm configuration module. */

static int alg_module_init(CONF_IMODULE *md, const CONF *cnf)
	{
	int i;
	const char *oid_section;
	STACK_OF(CONF_VALUE) *sktmp;
	CONF_VALUE *oval;
	oid_section = CONF_imodule_get_value(md);
	if(!(sktmp = NCONF_get_section(cnf, oid_section)))
		{
		EVPerr(EVP_F_ALG_MODULE_INIT, EVP_R_ERROR_LOADING_SECTION);
		return 0;
		}
	for(i = 0; i < sk_CONF_VALUE_num(sktmp); i++)
		{
		oval = sk_CONF_VALUE_value(sktmp, i);
		if (!strcmp(oval->name, "fips_mode"))
			{
			int m;
			if (!X509V3_get_value_bool(oval, &m))
				{
				EVPerr(EVP_F_ALG_MODULE_INIT, EVP_R_INVALID_FIPS_MODE);
				return 0;
				}
			if (m > 0)
				{
#ifdef OPENSSL_FIPS
				if (!FIPS_mode() && !FIPS_mode_set(1))
					{
					EVPerr(EVP_F_ALG_MODULE_INIT, EVP_R_ERROR_SETTING_FIPS_MODE);
					return 0;
					}
#else
				EVPerr(EVP_F_ALG_MODULE_INIT, EVP_R_FIPS_MODE_NOT_SUPPORTED);
				return 0;
#endif
				}
			}
		else
			{
			EVPerr(EVP_F_ALG_MODULE_INIT, EVP_R_UNKNOWN_OPTION);
			ERR_add_error_data(4, "name=", oval->name,
						", value=", oval->value);
			}
				
		}
	return 1;
	}

void EVP_add_alg_module(void)
	{
	CONF_module_add("alg_section", alg_module_init, 0);
	}