
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
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509.h>

static int init(EVP_MD_CTX *ctx)
	{ return 1; }

static int update(EVP_MD_CTX *ctx,const void *data,size_t count)
	{ return 1; }

static int final(EVP_MD_CTX *ctx,unsigned char *md)
	{ return 1; }

static const EVP_MD null_md=
	{
	NID_undef,
	NID_undef,
	0,
	0,
	init,
	update,
	final,
	NULL,
	NULL,
	EVP_PKEY_NULL_method,
	0,
	sizeof(EVP_MD *),
	};

const EVP_MD *EVP_md_null(void)
	{
	return(&null_md);
	}