
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

#include <common.h>
RCSID("$Id$");

krb5_error_code
store_string(krb5_storage *sp, const char *str)
{
    size_t len = strlen(str) + 1;
    krb5_error_code ret;

    ret = krb5_store_int32(sp, len);
    if (ret)
	return ret;
    ret = krb5_storage_write(sp, str, len);
    if (ret != len)
	return EINVAL;
    return 0;
}

static void
add_list(char ****list, size_t *listlen, char **str, size_t len)
{
    size_t i;
    *list = erealloc(*list, sizeof(**list) * (*listlen + 1));

    (*list)[*listlen] = ecalloc(len, sizeof(**list));
    for (i = 0; i < len; i++)
	(*list)[*listlen][i] = str[i];
    (*listlen)++;
}

static void
permute(char ****list, size_t *listlen,
	char **str, const int start, const int len)
{
    int i, j;

#define SWAP(s,i,j) { char *t = str[i]; str[i] = str[j]; str[j] = t; }

    for (i = start; i < len - 1; i++) {
	for (j = i+1; j < len; j++) {
	    SWAP(str,i,j);
	    permute(list, listlen, str, i+1, len);
	    SWAP(str,i,j);
	}
    }
    add_list(list, listlen, str, len);
}

char ***
permutate_all(struct getarg_strings *strings, size_t *size)
{
    char **list, ***all = NULL;
    int i;

    *size = 0;

    list = ecalloc(strings->num_strings, sizeof(*list));
    for (i = 0; i < strings->num_strings; i++)
	list[i] = strings->strings[i];

    permute(&all, size, list, 0, strings->num_strings);
    free(list);
    return all;
}