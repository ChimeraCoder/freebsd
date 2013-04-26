
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
#include <openssl/crypto.h>
#include <openssl/x509.h>

#ifndef OPENSSL_NO_STDIO
int X509_STORE_set_default_paths(X509_STORE *ctx)
	{
	X509_LOOKUP *lookup;

	lookup=X509_STORE_add_lookup(ctx,X509_LOOKUP_file());
	if (lookup == NULL) return(0);
	X509_LOOKUP_load_file(lookup,NULL,X509_FILETYPE_DEFAULT);

	lookup=X509_STORE_add_lookup(ctx,X509_LOOKUP_hash_dir());
	if (lookup == NULL) return(0);
	X509_LOOKUP_add_dir(lookup,NULL,X509_FILETYPE_DEFAULT);
	
	/* clear any errors */
	ERR_clear_error();

	return(1);
	}

int X509_STORE_load_locations(X509_STORE *ctx, const char *file,
		const char *path)
	{
	X509_LOOKUP *lookup;

	if (file != NULL)
		{
		lookup=X509_STORE_add_lookup(ctx,X509_LOOKUP_file());
		if (lookup == NULL) return(0);
		if (X509_LOOKUP_load_file(lookup,file,X509_FILETYPE_PEM) != 1)
		    return(0);
		}
	if (path != NULL)
		{
		lookup=X509_STORE_add_lookup(ctx,X509_LOOKUP_hash_dir());
		if (lookup == NULL) return(0);
		if (X509_LOOKUP_add_dir(lookup,path,X509_FILETYPE_PEM) != 1)
		    return(0);
		}
	if ((path == NULL) && (file == NULL))
		return(0);
	return(1);
	}

#endif