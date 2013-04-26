
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
#include <stdio.h>
#include <stdlib.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

#define MIN_WORDV_SIZE	32

/*
 * OpenPAM extension
 *
 * Read a line from a file and split it into words.
 */

char **
openpam_readlinev(FILE *f, int *lineno, int *lenp)
{
	char *word, **wordv, **tmp;
	size_t wordlen, wordvsize;
	int ch, serrno, wordvlen;

	wordvsize = MIN_WORDV_SIZE;
	wordvlen = 0;
	if ((wordv = malloc(wordvsize * sizeof *wordv)) == NULL) {
		openpam_log(PAM_LOG_ERROR, "malloc(): %m");
		errno = ENOMEM;
		return (NULL);
	}
	wordv[wordvlen] = NULL;
	while ((word = openpam_readword(f, lineno, &wordlen)) != NULL) {
		if ((unsigned int)wordvlen + 1 >= wordvsize) {
			/* need to expand the array */
			wordvsize *= 2;
			tmp = realloc(wordv, wordvsize * sizeof *wordv);
			if (tmp == NULL) {
				openpam_log(PAM_LOG_ERROR, "malloc(): %m");
				errno = ENOMEM;
				break;
			}
			wordv = tmp;
		}
		/* insert our word */
		wordv[wordvlen++] = word;
		wordv[wordvlen] = NULL;
	}
	if (errno != 0) {
		/* I/O error or out of memory */
		serrno = errno;
		while (wordvlen--)
			free(wordv[wordvlen]);
		free(wordv);
		errno = serrno;
		return (NULL);
	}
	/* assert(!ferror(f)) */
	ch = fgetc(f);
	/* assert(ch == EOF || ch == '\n') */
	if (ch == EOF && wordvlen == 0) {
		free(wordv);
		return (NULL);
	}
	if (ch == '\n' && lineno != NULL)
		++*lineno;
	if (lenp != NULL)
		*lenp = wordvlen;
	return (wordv);
}

/**
 * The =openpam_readlinev function reads a line from a file, splits it
 * into words according to the rules described in the =openpam_readword
 * manual page, and returns a list of those words.
 *
 * If =lineno is not =NULL, the integer variable it points to is
 * incremented every time a newline character is read.
 * This includes quoted or escaped newline characters and the newline
 * character at the end of the line.
 *
 * If =lenp is not =NULL, the number of words on the line is stored in the
 * variable to which it points.
 *
 * RETURN VALUES
 *
 * If successful, the =openpam_readlinev function returns a pointer to a
 * dynamically allocated array of pointers to individual dynamically
 * allocated NUL-terminated strings, each containing a single word, in the
 * order in which they were encountered on the line.
 * The array is terminated by a =NULL pointer.
 *
 * The caller is responsible for freeing both the array and the individual
 * strings by passing each of them to =!free.
 *
 * If the end of the line was reached before any words were read,
 * =openpam_readlinev returns a pointer to a dynamically allocated array
 * containing a single =NULL pointer.
 *
 * The =openpam_readlinev function can fail and return =NULL for one of
 * four reasons:
 *
 *  - The end of the file was reached before any words were read; :errno is
 *    zero, =!ferror returns zero, and =!feof returns a non-zero value.
 *
 *  - The end of the file was reached while a quote or backslash escape
 *    was in effect; :errno is set to =EINVAL, =!ferror returns zero, and
 *    =!feof returns a non-zero value.
 *
 *  - An error occurred while reading from the file; :errno is non-zero,
 *    =!ferror returns a non-zero value and =!feof returns zero.
 *
 *  - A =!malloc or =!realloc call failed; :errno is set to =ENOMEM,
 *    =!ferror returns a non-zero value, and =!feof may or may not return
 *    a non-zero value.
 *
 * >openpam_readline
 * >openpam_readword
 *
 * AUTHOR DES
 */