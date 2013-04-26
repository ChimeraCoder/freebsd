
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

#include "namespace.h"
#include <netdb.h>
#if defined(NLS)
#include <nl_types.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "reentrant.h"
#endif
#include "un-namespace.h"

/* Entries EAI_ADDRFAMILY (1) and EAI_NODATA (7) are obsoleted, but left */
/* for backward compatibility with userland code prior to 2553bis-02 */
static const char *ai_errlist[] = {
	"Success",					/* 0 */
	"Address family for hostname not supported",	/* 1 */
	"Temporary failure in name resolution",		/* EAI_AGAIN */
	"Invalid value for ai_flags",			/* EAI_BADFLAGS */
	"Non-recoverable failure in name resolution",	/* EAI_FAIL */
	"ai_family not supported",			/* EAI_FAMILY */
	"Memory allocation failure", 			/* EAI_MEMORY */
	"No address associated with hostname",		/* 7 */
	"hostname nor servname provided, or not known",	/* EAI_NONAME */
	"servname not supported for ai_socktype",	/* EAI_SERVICE */
	"ai_socktype not supported", 			/* EAI_SOCKTYPE */
	"System error returned in errno", 		/* EAI_SYSTEM */
	"Invalid value for hints",			/* EAI_BADHINTS */
	"Resolved protocol is unknown",			/* EAI_PROTOCOL */
	"Argument buffer overflow"			/* EAI_OVERFLOW */
};

#if defined(NLS)
static char		gai_buf[NL_TEXTMAX];
static once_t		gai_init_once = ONCE_INITIALIZER;
static thread_key_t	gai_key;
static int		gai_keycreated = 0;

static void
gai_keycreate(void)
{
	gai_keycreated = (thr_keycreate(&gai_key, free) == 0);
}
#endif

const char *
gai_strerror(int ecode)
{
#if defined(NLS)
	nl_catd catd;
	char *buf;

	if (thr_main() != 0)
		buf = gai_buf;
	else {
		if (thr_once(&gai_init_once, gai_keycreate) != 0 ||
		    !gai_keycreated)
			goto thr_err;
		if ((buf = thr_getspecific(gai_key)) == NULL) {
			if ((buf = malloc(sizeof(gai_buf))) == NULL)
				goto thr_err;
			if (thr_setspecific(gai_key, buf) != 0) {
				free(buf);
				goto thr_err;
			}
		}
	}

	catd = catopen("libc", NL_CAT_LOCALE);
	if (ecode > 0 && ecode < EAI_MAX)
		strlcpy(buf, catgets(catd, 3, ecode, ai_errlist[ecode]),
		    sizeof(gai_buf));
	else if (ecode == 0)
		strlcpy(buf, catgets(catd, 3, NL_MSGMAX - 1, "Success"),
		    sizeof(gai_buf));
	else
		strlcpy(buf, catgets(catd, 3, NL_MSGMAX, "Unknown error"),
		    sizeof(gai_buf));
	catclose(catd);
	return buf;

thr_err:
#endif
	if (ecode >= 0 && ecode < EAI_MAX)
		return ai_errlist[ecode];
	return "Unknown error";
}