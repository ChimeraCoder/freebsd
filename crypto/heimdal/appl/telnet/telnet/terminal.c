
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

#include "telnet_locl.h"

RCSID("$Id$");

Ring		ttyoring, ttyiring;
unsigned char	ttyobuf[2*BUFSIZ], ttyibuf[BUFSIZ];

int termdata;			/* Debugging flag */

# ifndef VDISCARD
cc_t termFlushChar;
# endif
# ifndef VLNEXT
cc_t termLiteralNextChar;
# endif
# ifndef VSUSP
cc_t termSuspChar;
# endif
# ifndef VWERASE
cc_t termWerasChar;
# endif
# ifndef VREPRINT
cc_t termRprntChar;
# endif
# ifndef VSTART
cc_t termStartChar;
# endif
# ifndef VSTOP
cc_t termStopChar;
# endif
# ifndef VEOL
cc_t termForw1Char;
# endif
# ifndef VEOL2
cc_t termForw2Char;
# endif
# ifndef VSTATUS
cc_t termAytChar;
# endif

/*
 * initialize the terminal data structures.
 */

void
init_terminal(void)
{
    if (ring_init(&ttyoring, ttyobuf, sizeof ttyobuf) != 1) {
	exit(1);
    }
    if (ring_init(&ttyiring, ttyibuf, sizeof ttyibuf) != 1) {
	exit(1);
    }
    autoflush = TerminalAutoFlush();
}


/*
 *		Send as much data as possible to the terminal.
 *
 *		Return value:
 *			-1: No useful work done, data waiting to go out.
 *			 0: No data was waiting, so nothing was done.
 *			 1: All waiting data was written out.
 *			 n: All data - n was written out.
 */


int
ttyflush(int drop)
{
    int n, n0, n1;

    n0 = ring_full_count(&ttyoring);
    if ((n1 = n = ring_full_consecutive(&ttyoring)) > 0) {
	if (drop) {
	    TerminalFlushOutput();
	    /* we leave 'n' alone! */
	} else {
	    n = TerminalWrite((char *)ttyoring.consume, n);
	}
    }
    if (n > 0) {
	if (termdata && n) {
	    Dump('>', ttyoring.consume, n);
	}
	/*
	 * If we wrote everything, and the full count is
	 * larger than what we wrote, then write the
	 * rest of the buffer.
	 */
	if (n1 == n && n0 > n) {
		n1 = n0 - n;
		if (!drop)
			n1 = TerminalWrite((char *)ttyoring.bottom, n1);
		if (n1 > 0)
			n += n1;
	}
	ring_consumed(&ttyoring, n);
    }
    if (n < 0)
	return -1;
    if (n == n0) {
	if (n0)
	    return -1;
	return 0;
    }
    return n0 - n + 1;
}


/*
 * These routines decides on what the mode should be (based on the values
 * of various global variables).
 */


int
getconnmode(void)
{
    int mode = 0;

    if (my_want_state_is_dont(TELOPT_ECHO))
	mode |= MODE_ECHO;

    if (localflow)
	mode |= MODE_FLOW;

    if ((eight & 1) || my_want_state_is_will(TELOPT_BINARY))
	mode |= MODE_INBIN;

    if (eight & 2)
	mode |= MODE_OUT8;
    if (his_want_state_is_will(TELOPT_BINARY))
	mode |= MODE_OUTBIN;

#ifdef	KLUDGELINEMODE
    if (kludgelinemode) {
	if (my_want_state_is_dont(TELOPT_SGA)) {
	    mode |= (MODE_TRAPSIG|MODE_EDIT);
	    if (dontlecho && (clocks.echotoggle > clocks.modenegotiated)) {
		mode &= ~MODE_ECHO;
	    }
	}
	return(mode);
    }
#endif
    if (my_want_state_is_will(TELOPT_LINEMODE))
	mode |= linemode;
    return(mode);
}

    void
setconnmode(force)
    int force;
{
#ifdef	ENCRYPTION
    static int enc_passwd = 0;
#endif
    int newmode;

    newmode = getconnmode()|(force?MODE_FORCE:0);

    TerminalNewMode(newmode);

#ifdef  ENCRYPTION
    if ((newmode & (MODE_ECHO|MODE_EDIT)) == MODE_EDIT) {
	if (my_want_state_is_will(TELOPT_ENCRYPT)
				&& (enc_passwd == 0) && !encrypt_output) {
	    encrypt_request_start(0, 0);
	    enc_passwd = 1;
	}
    } else {
	if (enc_passwd) {
	    encrypt_request_end();
	    enc_passwd = 0;
	}
    }
#endif

}


    void
setcommandmode()
{
    TerminalNewMode(-1);
}