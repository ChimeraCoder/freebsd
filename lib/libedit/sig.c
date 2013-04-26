
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

#if !defined(lint) && !defined(SCCSID)
static char sccsid[] = "@(#)sig.c	8.1 (Berkeley) 6/4/93";
#endif /* not lint && not SCCSID */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * sig.c: Signal handling stuff.
 *	  our policy is to trap all signals, set a good state
 *	  and pass the ball to our caller.
 */
#include "sys.h"
#include "el.h"
#include <stdlib.h>

private EditLine *sel = NULL;

private const int sighdl[] = {
#define	_DO(a)	(a),
	ALLSIGS
#undef	_DO
	- 1
};

private void sig_handler(int);

/* sig_handler():
 *	This is the handler called for all signals
 *	XXX: we cannot pass any data so we just store the old editline
 *	state in a private variable
 */
private void
sig_handler(int signo)
{
	int i;
	sigset_t nset, oset;

	(void) sigemptyset(&nset);
	(void) sigaddset(&nset, signo);
	(void) sigprocmask(SIG_BLOCK, &nset, &oset);

	sel->el_signal->sig_no = signo;

	switch (signo) {
	case SIGCONT:
		tty_rawmode(sel);
		if (ed_redisplay(sel, 0) == CC_REFRESH)
			re_refresh(sel);
		term__flush(sel);
		break;

	case SIGWINCH:
		el_resize(sel);
		break;

	default:
		tty_cookedmode(sel);
		break;
	}

	for (i = 0; sighdl[i] != -1; i++)
		if (signo == sighdl[i])
			break;

	(void) sigaction(signo, &sel->el_signal->sig_action[i], NULL);
	sel->el_signal->sig_action[i].sa_handler = SIG_ERR;
	sel->el_signal->sig_action[i].sa_flags = 0;
	sigemptyset(&sel->el_signal->sig_action[i].sa_mask);
	(void) sigprocmask(SIG_SETMASK, &oset, NULL);
	(void) kill(0, signo);
}


/* sig_init():
 *	Initialize all signal stuff
 */
protected int
sig_init(EditLine *el)
{
	size_t i;
	sigset_t *nset, oset;

	el->el_signal = el_malloc(sizeof(*el->el_signal));
	if (el->el_signal == NULL)
		return -1;

	nset = &el->el_signal->sig_set;
	(void) sigemptyset(nset);
#define	_DO(a) (void) sigaddset(nset, a);
	ALLSIGS
#undef	_DO
	(void) sigprocmask(SIG_BLOCK, nset, &oset);

	for (i = 0; sighdl[i] != -1; i++) {
		el->el_signal->sig_action[i].sa_handler = SIG_ERR;
		el->el_signal->sig_action[i].sa_flags = 0;
		sigemptyset(&el->el_signal->sig_action[i].sa_mask);
	}

	(void) sigprocmask(SIG_SETMASK, &oset, NULL);

	return 0;
}


/* sig_end():
 *	Clear all signal stuff
 */
protected void
sig_end(EditLine *el)
{

	el_free((ptr_t) el->el_signal);
	el->el_signal = NULL;
}


/* sig_set():
 *	set all the signal handlers
 */
protected void
sig_set(EditLine *el)
{
	size_t i;
	sigset_t oset;
	struct sigaction osa, nsa;

	nsa.sa_handler = sig_handler;
	nsa.sa_flags = 0;
	sigemptyset(&nsa.sa_mask);

	(void) sigprocmask(SIG_BLOCK, &el->el_signal->sig_set, &oset);

	for (i = 0; sighdl[i] != -1; i++) {
		/* This could happen if we get interrupted */
		if (sigaction(sighdl[i], &nsa, &osa) != -1 &&
		    osa.sa_handler != sig_handler)
			el->el_signal->sig_action[i] = osa;
	}
	sel = el;
	(void) sigprocmask(SIG_SETMASK, &oset, NULL);
}


/* sig_clr():
 *	clear all the signal handlers
 */
protected void
sig_clr(EditLine *el)
{
	size_t i;
	sigset_t oset;

	(void) sigprocmask(SIG_BLOCK, &el->el_signal->sig_set, &oset);

	for (i = 0; sighdl[i] != -1; i++)
		if (el->el_signal->sig_action[i].sa_handler != SIG_ERR)
			(void)sigaction(sighdl[i],
			    &el->el_signal->sig_action[i], NULL);

	sel = NULL;		/* we are going to die if the handler is
				 * called */
	(void)sigprocmask(SIG_SETMASK, &oset, NULL);
}