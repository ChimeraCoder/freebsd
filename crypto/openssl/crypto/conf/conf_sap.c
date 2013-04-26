
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
#include <openssl/crypto.h>
#include "cryptlib.h"
#include <openssl/conf.h>
#include <openssl/dso.h>
#include <openssl/x509.h>
#include <openssl/asn1.h>
#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

/* This is the automatic configuration loader: it is called automatically by
 * OpenSSL when any of a number of standard initialisation functions are called,
 * unless this is overridden by calling OPENSSL_no_config()
 */

static int openssl_configured = 0;

void OPENSSL_config(const char *config_name)
	{
	if (openssl_configured)
		return;

	OPENSSL_load_builtin_modules();
#ifndef OPENSSL_NO_ENGINE
	/* Need to load ENGINEs */
	ENGINE_load_builtin_engines();
#endif
	/* Add others here? */


	ERR_clear_error();
	if (CONF_modules_load_file(NULL, config_name,
	CONF_MFLAGS_DEFAULT_SECTION|CONF_MFLAGS_IGNORE_MISSING_FILE) <= 0)
		{
		BIO *bio_err;
		ERR_load_crypto_strings();
		if ((bio_err=BIO_new_fp(stderr, BIO_NOCLOSE)) != NULL)
			{
			BIO_printf(bio_err,"Auto configuration failed\n");
			ERR_print_errors(bio_err);
			BIO_free(bio_err);
			}
		exit(1);
		}

	return;
	}

void OPENSSL_no_config()
	{
	openssl_configured = 1;
	}