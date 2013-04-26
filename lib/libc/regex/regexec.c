
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
static char sccsid[] = "@(#)regexec.c	8.3 (Berkeley) 3/20/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * the outer shell of regexec()
 *
 * This file includes engine.c three times, after muchos fiddling with the
 * macros that code uses.  This lets the same code operate on two different
 * representations for state sets and characters.
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <regex.h>
#include <wchar.h>
#include <wctype.h>

#include "utils.h"
#include "regex2.h"

static int nope __unused = 0;	/* for use in asserts; shuts lint up */

static __inline size_t
xmbrtowc(wint_t *wi, const char *s, size_t n, mbstate_t *mbs, wint_t dummy)
{
	size_t nr;
	wchar_t wc;

	nr = mbrtowc(&wc, s, n, mbs);
	if (wi != NULL)
		*wi = wc;
	if (nr == 0)
		return (1);
	else if (nr == (size_t)-1 || nr == (size_t)-2) {
		memset(mbs, 0, sizeof(*mbs));
		if (wi != NULL)
			*wi = dummy;
		return (1);
	} else
                return (nr);
}

static __inline size_t
xmbrtowc_dummy(wint_t *wi,
		const char *s,
		size_t n __unused,
		mbstate_t *mbs __unused,
		wint_t dummy __unused)
{

	if (wi != NULL)
		*wi = (unsigned char)*s;
	return (1);
}

/* macros for manipulating states, small version */
#define	states	long
#define	states1	states		/* for later use in regexec() decision */
#define	CLEAR(v)	((v) = 0)
#define	SET0(v, n)	((v) &= ~((unsigned long)1 << (n)))
#define	SET1(v, n)	((v) |= (unsigned long)1 << (n))
#define	ISSET(v, n)	(((v) & ((unsigned long)1 << (n))) != 0)
#define	ASSIGN(d, s)	((d) = (s))
#define	EQ(a, b)	((a) == (b))
#define	STATEVARS	long dummy	/* dummy version */
#define	STATESETUP(m, n)	/* nothing */
#define	STATETEARDOWN(m)	/* nothing */
#define	SETUP(v)	((v) = 0)
#define	onestate	long
#define	INIT(o, n)	((o) = (unsigned long)1 << (n))
#define	INC(o)	((o) <<= 1)
#define	ISSTATEIN(v, o)	(((v) & (o)) != 0)
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#define	FWD(dst, src, n)	((dst) |= ((unsigned long)(src)&(here)) << (n))
#define	BACK(dst, src, n)	((dst) |= ((unsigned long)(src)&(here)) >> (n))
#define	ISSETBACK(v, n)	(((v) & ((unsigned long)here >> (n))) != 0)
/* no multibyte support */
#define	XMBRTOWC	xmbrtowc_dummy
#define	ZAPSTATE(mbs)	((void)(mbs))
/* function names */
#define SNAMES			/* engine.c looks after details */

#include "engine.c"

/* now undo things */
#undef	states
#undef	CLEAR
#undef	SET0
#undef	SET1
#undef	ISSET
#undef	ASSIGN
#undef	EQ
#undef	STATEVARS
#undef	STATESETUP
#undef	STATETEARDOWN
#undef	SETUP
#undef	onestate
#undef	INIT
#undef	INC
#undef	ISSTATEIN
#undef	FWD
#undef	BACK
#undef	ISSETBACK
#undef	SNAMES
#undef	XMBRTOWC
#undef	ZAPSTATE

/* macros for manipulating states, large version */
#define	states	char *
#define	CLEAR(v)	memset(v, 0, m->g->nstates)
#define	SET0(v, n)	((v)[n] = 0)
#define	SET1(v, n)	((v)[n] = 1)
#define	ISSET(v, n)	((v)[n])
#define	ASSIGN(d, s)	memcpy(d, s, m->g->nstates)
#define	EQ(a, b)	(memcmp(a, b, m->g->nstates) == 0)
#define	STATEVARS	long vn; char *space
#define	STATESETUP(m, nv)	{ (m)->space = malloc((nv)*(m)->g->nstates); \
				if ((m)->space == NULL) return(REG_ESPACE); \
				(m)->vn = 0; }
#define	STATETEARDOWN(m)	{ free((m)->space); }
#define	SETUP(v)	((v) = &m->space[m->vn++ * m->g->nstates])
#define	onestate	long
#define	INIT(o, n)	((o) = (n))
#define	INC(o)	((o)++)
#define	ISSTATEIN(v, o)	((v)[o])
/* some abbreviations; note that some of these know variable names! */
/* do "if I'm here, I can also be there" etc without branches */
#define	FWD(dst, src, n)	((dst)[here+(n)] |= (src)[here])
#define	BACK(dst, src, n)	((dst)[here-(n)] |= (src)[here])
#define	ISSETBACK(v, n)	((v)[here - (n)])
/* no multibyte support */
#define	XMBRTOWC	xmbrtowc_dummy
#define	ZAPSTATE(mbs)	((void)(mbs))
/* function names */
#define	LNAMES			/* flag */

#include "engine.c"

/* multibyte character & large states version */
#undef	LNAMES
#undef	XMBRTOWC
#undef	ZAPSTATE
#define	XMBRTOWC	xmbrtowc
#define	ZAPSTATE(mbs)	memset((mbs), 0, sizeof(*(mbs)))
#define	MNAMES

#include "engine.c"

/*
 - regexec - interface for matching
 = extern int regexec(const regex_t *, const char *, size_t, \
 =					regmatch_t [], int);
 = #define	REG_NOTBOL	00001
 = #define	REG_NOTEOL	00002
 = #define	REG_STARTEND	00004
 = #define	REG_TRACE	00400	// tracing of execution
 = #define	REG_LARGE	01000	// force large representation
 = #define	REG_BACKR	02000	// force use of backref code
 *
 * We put this here so we can exploit knowledge of the state representation
 * when choosing which matcher to call.  Also, by this point the matchers
 * have been prototyped.
 */
int				/* 0 success, REG_NOMATCH failure */
regexec(const regex_t * __restrict preg,
	const char * __restrict string,
	size_t nmatch,
	regmatch_t pmatch[__restrict],
	int eflags)
{
	struct re_guts *g = preg->re_g;
#ifdef REDEBUG
#	define	GOODFLAGS(f)	(f)
#else
#	define	GOODFLAGS(f)	((f)&(REG_NOTBOL|REG_NOTEOL|REG_STARTEND))
#endif

	if (preg->re_magic != MAGIC1 || g->magic != MAGIC2)
		return(REG_BADPAT);
	assert(!(g->iflags&BAD));
	if (g->iflags&BAD)		/* backstop for no-debug case */
		return(REG_BADPAT);
	eflags = GOODFLAGS(eflags);

	if (MB_CUR_MAX > 1)
		return(mmatcher(g, (char *)string, nmatch, pmatch, eflags));
	else if (g->nstates <= CHAR_BIT*sizeof(states1) && !(eflags&REG_LARGE))
		return(smatcher(g, (char *)string, nmatch, pmatch, eflags));
	else
		return(lmatcher(g, (char *)string, nmatch, pmatch, eflags));
}