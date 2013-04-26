
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
#include "openpam_ctype.h"

#define MIN_WORD_SIZE	32

/*
 * OpenPAM extension
 *
 * Read a word from a file, respecting shell quoting rules.
 */

char *
openpam_readword(FILE *f, int *lineno, size_t *lenp)
{
	char *word;
	size_t size, len;
	int ch, comment, escape, quote;
	int serrno;

	errno = 0;

	/* skip initial whitespace */
	comment = 0;
	while ((ch = getc(f)) != EOF && ch != '\n') {
		if (ch == '#')
			comment = 1;
		if (!is_lws(ch) && !comment)
			break;
	}
	if (ch == EOF)
		return (NULL);
	ungetc(ch, f);
	if (ch == '\n')
		return (NULL);

	word = NULL;
	size = len = 0;
	escape = quote = 0;
	while ((ch = fgetc(f)) != EOF && (!is_ws(ch) || quote || escape)) {
		if (ch == '\\' && !escape && quote != '\'') {
			/* escape next character */
			escape = ch;
		} else if ((ch == '\'' || ch == '"') && !quote && !escape) {
			/* begin quote */
			quote = ch;
			/* edge case: empty quoted string */
			if (openpam_straddch(&word, &size, &len, 0) != 0)
				return (NULL);
		} else if (ch == quote && !escape) {
			/* end quote */
			quote = 0;
		} else if (ch == '\n' && escape && quote != '\'') {
			/* line continuation */
			escape = 0;
		} else {
			if (escape && quote && ch != '\\' && ch != quote &&
			    openpam_straddch(&word, &size, &len, '\\') != 0) {
				free(word);
				errno = ENOMEM;
				return (NULL);
			}
			if (openpam_straddch(&word, &size, &len, ch) != 0) {
				free(word);
				errno = ENOMEM;
				return (NULL);
			}
			escape = 0;
		}
		if (lineno != NULL && ch == '\n')
			++*lineno;
	}
	if (ch == EOF && ferror(f)) {
		serrno = errno;
		free(word);
		errno = serrno;
		return (NULL);
	}
	if (ch == EOF && (escape || quote)) {
		/* Missing escaped character or closing quote. */
		openpam_log(PAM_LOG_ERROR, "unexpected end of file");
		free(word);
		errno = EINVAL;
		return (NULL);
	}
	ungetc(ch, f);
	if (lenp != NULL)
		*lenp = len;
	return (word);
}

/**
 * The =openpam_readword function reads the next word from a file, and
 * returns it in a NUL-terminated buffer allocated with =!malloc.
 *
 * A word is a sequence of non-whitespace characters.
 * However, whitespace characters can be included in a word if quoted or
 * escaped according to the following rules:
 *
 *  - An unescaped single or double quote introduces a quoted string,
 *    which ends when the same quote character is encountered a second
 *    time.
 *    The quotes themselves are stripped.
 *
 *  - Within a single- or double-quoted string, all whitespace characters,
 *    including the newline character, are preserved as-is.
 *
 *  - Outside a quoted string, a backslash escapes the next character,
 *    which is preserved as-is, unless that character is a newline, in
 *    which case it is discarded and reading continues at the beginning of
 *    the next line as if the backslash and newline had not been there.
 *    In all cases, the backslash itself is discarded.
 *
 *  - Within a single-quoted string, double quotes and backslashes are
 *    preserved as-is.
 *
 *  - Within a double-quoted string, a single quote is preserved as-is,
 *    and a backslash is preserved as-is unless used to escape a double
 *    quote.
 *
 * In addition, if the first non-whitespace character on the line is a
 * hash character (#), the rest of the line is discarded.
 * If a hash character occurs within a word, however, it is preserved
 * as-is.
 * A backslash at the end of a comment does cause line continuation.
 *
 * If =lineno is not =NULL, the integer variable it points to is
 * incremented every time a quoted or escaped newline character is read.
 *
 * If =lenp is not =NULL, the length of the word (after quotes and
 * backslashes have been removed) is stored in the variable it points to.
 *
 * RETURN VALUES
 *
 * If successful, the =openpam_readword function returns a pointer to a
 * dynamically allocated NUL-terminated string containing the first word
 * encountered on the line.
 *
 * The caller is responsible for releasing the returned buffer by passing
 * it to =!free.
 *
 * If =openpam_readword reaches the end of the line or file before any
 * characters are copied to the word, it returns =NULL.  In the former
 * case, the newline is pushed back to the file.
 *
 * If =openpam_readword reaches the end of the file while a quote or
 * backslash escape is in effect, it sets :errno to =EINVAL and returns
 * =NULL.
 *
 * IMPLEMENTATION NOTES
 *
 * The parsing rules are intended to be equivalent to the normal POSIX
 * shell quoting rules.
 * Any discrepancy is a bug and should be reported to the author along
 * with sample input that can be used to reproduce the error.
 *
 * >openpam_readline
 * >openpam_readlinev
 *
 * AUTHOR DES
 */