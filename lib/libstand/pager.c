
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "stand.h"
#include <string.h>

static int	p_maxlines = -1;
static int	p_freelines;

static char *pager_prompt1 = " --more--  <space> page down <enter> line down <q> quit ";
static char *pager_blank   = "                                                        ";

/*
 * 'open' the pager
 */
void
pager_open(void)
{
    int		nlines;
    char	*cp, *lp;
    
    nlines = 24;		/* sensible default */
    if ((cp = getenv("LINES")) != NULL) {
	nlines = strtol(cp, &lp, 0);
    }

    p_maxlines = nlines - 1;
    if (p_maxlines < 1)
	p_maxlines = 1;
    p_freelines = p_maxlines;
}

/*
 * 'close' the pager
 */
void
pager_close(void)
{
    p_maxlines = -1;
}

/*
 * Emit lines to the pager; may not return until the user
 * has responded to the prompt.
 *
 * Will return nonzero if the user enters 'q' or 'Q' at the prompt.
 *
 * XXX note that this watches outgoing newlines (and eats them), but
 *     does not handle wrap detection (req. count of columns).
 */

int
pager_output(const char *cp)
{
    int		action;

    if (cp == NULL)
	return(0);
    
    for (;;) {
	if (*cp == 0)
	    return(0);
	
	putchar(*cp);			/* always emit character */

	if (*(cp++) == '\n') {		/* got a newline? */
	    p_freelines--;
	    if (p_freelines <= 0) {
		printf("%s", pager_prompt1);
		action = 0;
		while (action == 0) {
		    switch(getchar()) {
		    case '\r':
		    case '\n':
			p_freelines = 1;
			action = 1;
			break;
		    case ' ':
			p_freelines = p_maxlines;
			action = 1;
			break;
		    case 'q':
		    case 'Q':
			action = 2;
			break;
		    default:
			break;
		    }
		}
		printf("\r%s\r", pager_blank);
		if (action == 2)
		    return(1);
	    }
	}
    }
}

/*
 * Display from (fd).
 */
int
pager_file(const char *fname)
{
    char	buf[80];
    size_t	hmuch;
    int		fd;
    int		result;
    
    if ((fd = open(fname, O_RDONLY)) == -1) {
	printf("can't open '%s': %s\n", fname, strerror(errno));
	return(-1);
    }

    for (;;) {
	hmuch = read(fd, buf, sizeof(buf) - 1);
	if (hmuch == -1) {
	    result = -1;
	    break;
	}
	if (hmuch == 0) {
	    result = 0;
	    break;
	}
	buf[hmuch] = 0;
	if (pager_output(buf)) {
	    result = 1;
	    break;
	}
    }
    close(fd);
    return(result);
}