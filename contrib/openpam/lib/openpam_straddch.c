
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <stdlib.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

#define MIN_STR_SIZE	32

/*
 * OpenPAM extension
 *
 * Add a character to a string, expanding the buffer if needed.
 */

int
openpam_straddch(char **str, size_t *size, size_t *len, int ch)
{
	size_t tmpsize;
	char *tmpstr;

	if (*str == NULL) {
		/* initial allocation */
		tmpsize = MIN_STR_SIZE;
		if ((tmpstr = malloc(tmpsize)) == NULL) {
			openpam_log(PAM_LOG_ERROR, "malloc(): %m");
			errno = ENOMEM;
			return (-1);
		}
		*str = tmpstr;
		*size = tmpsize;
		*len = 0;
	} else if (*len + 1 >= *size) {
		/* additional space required */
		tmpsize = *size * 2;
		if ((tmpstr = realloc(*str, tmpsize)) == NULL) {
			openpam_log(PAM_LOG_ERROR, "realloc(): %m");
			errno = ENOMEM;
			return (-1);
		}
		*size = tmpsize;
		*str = tmpstr;
	}
	(*str)[*len] = ch;
	++*len;
	(*str)[*len] = '\0';
	return (0);
}

/**
 * The =openpam_straddch function appends a character to a dynamically
 * allocated NUL-terminated buffer, reallocating the buffer as needed.
 *
 * The =str argument points to a variable containing either a pointer to
 * an existing buffer or =NULL.
 * If the value of the variable pointed to by =str is =NULL, a new buffer
 * is allocated.
 *
 * The =size and =len argument point to variables used to hold the size
 * of the buffer and the length of the string it contains, respectively.
 *
 * If a new buffer is allocated or an existing buffer is reallocated to
 * make room for the additional character, =str and =size are updated
 * accordingly.
 *
 * The =openpam_straddch function ensures that the buffer is always
 * NUL-terminated.
 *
 * If the =openpam_straddch function is successful, it increments the
 * integer variable pointed to by =len and returns 0.
 * Otherwise, it leaves the variables pointed to by =str, =size and =len
 * unmodified, sets :errno to =ENOMEM and returns -1.
 *
 * AUTHOR DES
 */