
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
static char sccsid[] = "@(#)strerror.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#if defined(NLS)
#include <nl_types.h>
#endif
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "reentrant.h"
#include "un-namespace.h"

#define	UPREFIX		"Unknown signal"

static char		sig_ebuf[NL_TEXTMAX];
static char		sig_ebuf_err[NL_TEXTMAX];
static once_t		sig_init_once = ONCE_INITIALIZER;
static thread_key_t	sig_key;
static int		sig_keycreated = 0;

static void
sig_keycreate(void)
{
	sig_keycreated = (thr_keycreate(&sig_key, free) == 0);
}

static char *
sig_tlsalloc(void)
{
	char *ebuf = NULL;

	if (thr_main() != 0)
		ebuf = sig_ebuf;
	else {
		if (thr_once(&sig_init_once, sig_keycreate) != 0 ||
		    !sig_keycreated)
			goto thr_err;
		if ((ebuf = thr_getspecific(sig_key)) == NULL) {
			if ((ebuf = malloc(sizeof(sig_ebuf))) == NULL)
				goto thr_err;
			if (thr_setspecific(sig_key, ebuf) != 0) {
				free(ebuf);
				ebuf = NULL;
				goto thr_err;
			}
		}
	}
thr_err:
	if (ebuf == NULL)
		ebuf = sig_ebuf_err;
	return (ebuf);
}

/* XXX: negative 'num' ? (REGR) */
char *
strsignal(int num)
{
	char *ebuf;
	char tmp[20];
	size_t n;
	int signum;
	char *t, *p;

#if defined(NLS)
	int saved_errno = errno;
	nl_catd catd;
	catd = catopen("libc", NL_CAT_LOCALE);
#endif

	ebuf = sig_tlsalloc();

	if (num > 0 && num < sys_nsig) {
		n = strlcpy(ebuf,
#if defined(NLS)
			catgets(catd, 2, num, sys_siglist[num]),
#else
			sys_siglist[num],
#endif
			sizeof(sig_ebuf));
	} else {
		n = strlcpy(ebuf,
#if defined(NLS)
			catgets(catd, 2, 0xffff, UPREFIX),
#else
			UPREFIX,
#endif
			sizeof(sig_ebuf));

		signum = num;
		if (num < 0)
			signum = -signum;

		t = tmp;
		do {
			*t++ = "0123456789"[signum % 10];
		} while (signum /= 10);
		if (num < 0)
			*t++ = '-';

		p = (ebuf + n);
		*p++ = ':';
		*p++ = ' ';

		for (;;) {
			*p++ = *--t;
			if (t <= tmp)
				break;
		}
		*p = '\0';
	}

#if defined(NLS)
	catclose(catd);
	errno = saved_errno;
#endif
	return (ebuf);
}