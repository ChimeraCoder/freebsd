
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

#include "hx_locl.h"


static int
null_init(hx509_context context,
	  hx509_certs certs, void **data, int flags,
	  const char *residue, hx509_lock lock)
{
    *data = NULL;
    return 0;
}

static int
null_free(hx509_certs certs, void *data)
{
    assert(data == NULL);
    return 0;
}

static int
null_iter_start(hx509_context context,
		hx509_certs certs, void *data, void **cursor)
{
    *cursor = NULL;
    return 0;
}

static int
null_iter(hx509_context context,
	  hx509_certs certs, void *data, void *iter, hx509_cert *cert)
{
    *cert = NULL;
    return ENOENT;
}

static int
null_iter_end(hx509_context context,
	      hx509_certs certs,
	      void *data,
	      void *cursor)
{
    assert(cursor == NULL);
    return 0;
}


struct hx509_keyset_ops keyset_null = {
    "NULL",
    0,
    null_init,
    NULL,
    null_free,
    NULL,
    NULL,
    null_iter_start,
    null_iter,
    null_iter_end
};

void
_hx509_ks_null_register(hx509_context context)
{
    _hx509_ks_register(context, &keyset_null);
}