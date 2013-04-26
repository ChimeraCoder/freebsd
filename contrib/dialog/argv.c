
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

#include <dialog.h>
#include <string.h>

/*
 * Convert a string to an argv[], returning a char** index (which must be
 * freed by the caller).  The string is modified (replacing gaps between
 * tokens with nulls).
 */
char **
dlg_string_to_argv(char *blob)
{
    size_t n;
    int pass;
    size_t length = strlen(blob);
    char **result = 0;

    for (pass = 0; pass < 2; ++pass) {
	bool inparm = FALSE;
	bool quoted = FALSE;
	char *param = blob;
	size_t count = 0;

	for (n = 0; n < length; ++n) {
	    if (quoted && blob[n] == '"') {
		quoted = FALSE;
	    } else if (blob[n] == '"') {
		quoted = TRUE;
		if (!inparm) {
		    if (pass)
			result[count] = param;
		    ++count;
		    inparm = TRUE;
		}
	    } else if (blob[n] == '\\') {
		if (quoted && !isspace(UCH(blob[n + 1]))) {
		    if (!inparm) {
			if (pass)
			    result[count] = param;
			++count;
			inparm = TRUE;
		    }
		    if (pass) {
			*param++ = blob[n];
			*param++ = blob[n + 1];
		    }
		}
		++n;
	    } else if (!quoted && isspace(UCH(blob[n]))) {
		inparm = FALSE;
		if (pass) {
		    *param++ = '\0';
		}
	    } else {
		if (!inparm) {
		    if (pass)
			result[count] = param;
		    ++count;
		    inparm = TRUE;
		}
		if (pass) {
		    *param++ = blob[n];
		}
	    }
	}

	if (!pass) {
	    if (count) {
		result = dlg_calloc(char *, count + 1);
		assert_ptr(result, "string_to_argv");
	    } else {
		break;		/* no tokens found */
	    }
	} else {
	    *param = '\0';
	}
    }
    return result;
}

/*
 * Count the entries in an argv list.
 */
int
dlg_count_argv(char **argv)
{
    int result = 0;

    if (argv != 0) {
	while (argv[result] != 0)
	    ++result;
    }
    return result;
}

int
dlg_eat_argv(int *argcp, char ***argvp, int start, int count)
{
    int k;

    *argcp -= count;
    for (k = start; k <= *argcp; k++)
	(*argvp)[k] = (*argvp)[k + count];
    (*argvp)[*argcp] = 0;
    return TRUE;
}