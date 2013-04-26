
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

#include "kadm5_locl.h"

RCSID("$Id$");

/*
 * free all the memory used by (len, keys)
 */

void
_kadm5_free_keys (krb5_context context,
		  int len, Key *keys)
{
    hdb_free_keys(context, len, keys);
}

/*
 * null-ify `len', `keys'
 */

void
_kadm5_init_keys (Key *keys, int len)
{
    int i;

    for (i = 0; i < len; ++i) {
	keys[i].mkvno               = NULL;
	keys[i].salt                = NULL;
	keys[i].key.keyvalue.length = 0;
	keys[i].key.keyvalue.data   = NULL;
    }
}

/*
 * return 1 if any key in `keys1, len1' exists in `keys2, len2'
 */

int
_kadm5_exists_keys(Key *keys1, int len1, Key *keys2, int len2)
{
    int i, j;

    for (i = 0; i < len1; ++i) {
	for (j = 0; j < len2; j++) {
	    if ((keys1[i].salt != NULL && keys2[j].salt == NULL)
		|| (keys1[i].salt == NULL && keys2[j].salt != NULL))
		continue;

	    if (keys1[i].salt != NULL) {
		if (keys1[i].salt->type != keys2[j].salt->type)
		    continue;
		if (keys1[i].salt->salt.length != keys2[j].salt->salt.length)
		    continue;
		if (memcmp (keys1[i].salt->salt.data, keys2[j].salt->salt.data,
			    keys1[i].salt->salt.length) != 0)
		    continue;
	    }
	    if (keys1[i].key.keytype != keys2[j].key.keytype)
		continue;
	    if (keys1[i].key.keyvalue.length != keys2[j].key.keyvalue.length)
		continue;
	    if (memcmp (keys1[i].key.keyvalue.data, keys2[j].key.keyvalue.data,
			keys1[i].key.keyvalue.length) != 0)
		continue;

	    return 1;
	}
    }
    return 0;
}