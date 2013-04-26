
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


#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <sys/param.h>
#include <arpa/inet.h>
#include <stdio.h>

#include "locate.h"

char 	**colon(char **, char*, char*);
char 	*patprep(char *);
void print_matches(u_int);
u_char 	*tolower_word(u_char *);
int 	getwm(caddr_t);
int 	getwf(FILE *);
int	check_bigram_char(int);

/* 
 * Validate bigram chars. If the test failed the database is corrupt 
 * or the database is obviously not a locate database.
 */
int
check_bigram_char(ch)
	int ch;
{
	/* legal bigram: 0, ASCII_MIN ... ASCII_MAX */
	if (ch == 0 ||
	    (ch >= ASCII_MIN && ch <= ASCII_MAX))
		return(ch);

	errx(1,
		"locate database header corrupt, bigram char outside 0, %d-%d: %d",  
		ASCII_MIN, ASCII_MAX, ch);
	exit(1);
}

/* split a colon separated string into a char vector
 *
 * "bla:foo" -> {"foo", "bla"}
 * "bla:"    -> {"foo", dot}
 * "bla"     -> {"bla"}
 * ""	     -> do nothing
 *
 */
char **
colon(dbv, path, dot)
	char **dbv;
	char *path;
	char *dot; /* default for single ':' */
{
	int vlen, slen;
	char *c, *ch, *p;
	char **pv;

	if (dbv == NULL) {
		if ((dbv = malloc(sizeof(char **))) == NULL)
			err(1, "malloc");
		*dbv = NULL;
	}

	/* empty string */
	if (*path == '\0') {
		warnx("empty database name, ignored");
		return(dbv);
	}

	/* length of string vector */
	for(vlen = 0, pv = dbv; *pv != NULL; pv++, vlen++);

	for (ch = c = path; ; ch++) {
		if (*ch == ':' ||
		    (!*ch && !(*(ch - 1) == ':' && ch == 1+ path))) {
			/* single colon -> dot */
			if (ch == c)
				p = dot;
			else {
				/* a string */
				slen = ch - c;
				if ((p = malloc(sizeof(char) * (slen + 1))) 
				    == NULL)
					err(1, "malloc");
				bcopy(c, p, slen);
				*(p + slen) = '\0';
			}
			/* increase dbv with element p */
			if ((dbv = realloc(dbv, sizeof(char **) * (vlen + 2)))
			    == NULL)
				err(1, "realloc");
			*(dbv + vlen) = p;
			*(dbv + ++vlen) = NULL;
			c = ch + 1;
		}
		if (*ch == '\0')
			break;
	}
	return (dbv);
}

void 
print_matches(counter)
	u_int counter;
{
	(void)printf("%d\n", counter);
}


/*
 * extract last glob-free subpattern in name for fast pre-match; prepend
 * '\0' for backwards match; return end of new pattern
 */
static char globfree[100];

char *
patprep(name)
	char *name;
{
	register char *endmark, *p, *subp;

	subp = globfree;
	*subp++ = '\0';   /* set first element to '\0' */
	p = name + strlen(name) - 1;

	/* skip trailing metacharacters */
	for (; p >= name; p--)
		if (strchr(LOCATE_REG, *p) == NULL)
			break;

	/* 
	 * check if maybe we are in a character class
	 *
	 * 'foo.[ch]'
	 *        |----< p
	 */
	if (p >= name && 
	    (strchr(p, '[') != NULL || strchr(p, ']') != NULL)) {
		for (p = name; *p != '\0'; p++)
			if (*p == ']' || *p == '[')
				break;
		p--;

		/* 
		 * cannot find a non-meta character, give up
		 * '*\*[a-z]'
		 *    |-------< p
		 */
		if (p >= name && strchr(LOCATE_REG, *p) != NULL)
			p = name - 1;
	}
	
	if (p < name) 			
		/* only meta chars: "???", force '/' search */
		*subp++ = '/';

	else {
		for (endmark = p; p >= name; p--)
			if (strchr(LOCATE_REG, *p) != NULL)
				break;
		for (++p;
		    (p <= endmark) && subp < (globfree + sizeof(globfree));)
			*subp++ = *p++;
	}
	*subp = '\0';
	return(--subp);
}

/* tolower word */
u_char *
tolower_word(word)
	u_char *word;
{
	register u_char *p;

	for(p = word; *p != '\0'; p++)
		*p = TOLOWER(*p);

	return(word);
}


/*
 * Read integer from mmap pointer.
 * Essentially a simple ``return *(int *)p'' but avoids sigbus
 * for integer alignment (SunOS 4.x, 5.x).
 *
 * Convert network byte order to host byte order if necessary.
 * So we can read a locate database on FreeBSD/i386 (little endian)
 * which was built on SunOS/sparc (big endian).
 */

int
getwm(p)
	caddr_t p;
{
	union {
		char buf[INTSIZE];
		int i;
	} u;
	register int i;

	for (i = 0; i < (int)INTSIZE; i++)
		u.buf[i] = *p++;

	i = u.i;

	if (i > MAXPATHLEN || i < -(MAXPATHLEN)) {
		i = ntohl(i);
		if (i > MAXPATHLEN || i < -(MAXPATHLEN))
			errx(1, "integer out of +-MAXPATHLEN (%d): %u",
			    MAXPATHLEN, abs(i) < abs(htonl(i)) ? i : htonl(i));
	}
	return(i);
}

/*
 * Read integer from stream.
 *
 * Convert network byte order to host byte order if necessary.
 * So we can read on FreeBSD/i386 (little endian) a locate database
 * which was built on SunOS/sparc (big endian).
 */

int
getwf(fp)
	FILE *fp;
{
	register int word;

	word = getw(fp);

	if (word > MAXPATHLEN || word < -(MAXPATHLEN)) {
		word = ntohl(word);
		if (word > MAXPATHLEN || word < -(MAXPATHLEN))
			errx(1, "integer out of +-MAXPATHLEN (%d): %u",
			    MAXPATHLEN, abs(word) < abs(htonl(word)) ? word :
				htonl(word));
	}
	return(word);
}