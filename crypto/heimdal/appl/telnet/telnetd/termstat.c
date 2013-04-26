
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

#include "telnetd.h"

RCSID("$Id$");

/*
 * local variables
 */
int def_tspeed = -1, def_rspeed = -1;
#ifdef	TIOCSWINSZ
int def_row = 0, def_col = 0;
#endif

/*
 * flowstat
 *
 * Check for changes to flow control
 */
void
flowstat(void)
{
    if (his_state_is_will(TELOPT_LFLOW)) {
	if (tty_flowmode() != flowmode) {
	    flowmode = tty_flowmode();
	    output_data("%c%c%c%c%c%c",
			IAC, SB, TELOPT_LFLOW,
			flowmode ? LFLOW_ON : LFLOW_OFF,
			IAC, SE);
	}
	if (tty_restartany() != restartany) {
	    restartany = tty_restartany();
	    output_data("%c%c%c%c%c%c",
			IAC, SB, TELOPT_LFLOW,
			restartany ? LFLOW_RESTART_ANY
			: LFLOW_RESTART_XON,
			IAC, SE);
	}
    }
}

/*
 * clientstat
 *
 * Process linemode related requests from the client.
 * Client can request a change to only one of linemode, editmode or slc's
 * at a time, and if using kludge linemode, then only linemode may be
 * affected.
 */
void
clientstat(int code, int parm1, int parm2)
{
    /*
     * Get a copy of terminal characteristics.
     */
    init_termbuf();

    /*
     * Process request from client. code tells what it is.
     */
    switch (code) {
    case TELOPT_NAWS:
#ifdef	TIOCSWINSZ
	{
	    struct winsize ws;

	    def_col = parm1;
	    def_row = parm2;

	    /*
	     * Change window size as requested by client.
	     */

	    ws.ws_col = parm1;
	    ws.ws_row = parm2;
	    ioctl(ourpty, TIOCSWINSZ, (char *)&ws);
	}
#endif	/* TIOCSWINSZ */

    break;

    case TELOPT_TSPEED:
	{
	    def_tspeed = parm1;
	    def_rspeed = parm2;
	    /*
	     * Change terminal speed as requested by client.
	     * We set the receive speed first, so that if we can't
	     * store seperate receive and transmit speeds, the transmit
	     * speed will take precedence.
	     */
	    tty_rspeed(parm2);
	    tty_tspeed(parm1);
	    set_termbuf();

	    break;

	}  /* end of case TELOPT_TSPEED */

    default:
	/* What? */
	break;
    }  /* end of switch */

    netflush();

}