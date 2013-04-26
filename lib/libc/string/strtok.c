
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strtok.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <stddef.h>
#ifdef DEBUG_STRTOK
#include <stdio.h>
#endif
#include <string.h>

char	*__strtok_r(char *, const char *, char **);

__weak_reference(__strtok_r, strtok_r);

char *
__strtok_r(char *s, const char *delim, char **last)
{
	char *spanp, *tok;
	int c, sc;

	if (s == NULL && (s = *last) == NULL)
		return (NULL);

	/*
	 * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
	 */
cont:
	c = *s++;
	for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}

	if (c == 0) {		/* no non-delimiter characters */
		*last = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = (char *)delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = '\0';
				*last = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}

char *
strtok(char *s, const char *delim)
{
	static char *last;

	return (__strtok_r(s, delim, &last));
}

#ifdef DEBUG_STRTOK
/*
 * Test the tokenizer.
 */
int
main(void)
{
	char blah[80], test[80];
	char *brkb, *brkt, *phrase, *sep, *word;

	sep = "\\/:;=-";
	phrase = "foo";

	printf("String tokenizer test:\n");
	strcpy(test, "This;is.a:test:of=the/string\\tokenizer-function.");
	for (word = strtok(test, sep); word; word = strtok(NULL, sep))
		printf("Next word is \"%s\".\n", word);
	strcpy(test, "This;is.a:test:of=the/string\\tokenizer-function.");

	for (word = strtok_r(test, sep, &brkt); word;
	    word = strtok_r(NULL, sep, &brkt)) {
		strcpy(blah, "blah:blat:blab:blag");

		for (phrase = strtok_r(blah, sep, &brkb); phrase;
		    phrase = strtok_r(NULL, sep, &brkb))
			printf("So far we're at %s:%s\n", word, phrase);
	}

	return (0);
}

#endif /* DEBUG_STRTOK */