
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

/****************************************************************************
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1992,1995               *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 ****************************************************************************/

#include <curses.priv.h>

#include <term.h>

MODULE_ID("$Id: lib_print.c,v 1.16 2006/11/26 00:26:34 tom Exp $")

NCURSES_EXPORT(int)
mcprint(char *data, int len)
/* ship binary character data to the printer via mc4/mc5/mc5p */
{
    char *mybuf, *switchon;
    size_t onsize, offsize, res;

    errno = 0;
    if (!cur_term || (!prtr_non && (!prtr_on || !prtr_off))) {
	errno = ENODEV;
	return (ERR);
    }

    if (prtr_non) {
	switchon = TPARM_1(prtr_non, len);
	onsize = strlen(switchon);
	offsize = 0;
    } else {
	switchon = prtr_on;
	onsize = strlen(prtr_on);
	offsize = strlen(prtr_off);
    }

    if (switchon == 0
	|| (mybuf = typeMalloc(char, onsize + len + offsize + 1)) == 0) {
	errno = ENOMEM;
	return (ERR);
    }

    (void) strcpy(mybuf, switchon);
    memcpy(mybuf + onsize, data, (unsigned) len);
    if (offsize)
	(void) strcpy(mybuf + onsize + len, prtr_off);

    /*
     * We're relying on the atomicity of UNIX writes here.  The
     * danger is that output from a refresh() might get interspersed
     * with the printer data after the write call returns but before the
     * data has actually been shipped to the terminal.  If the write(2)
     * operation is truly atomic we're protected from this.
     */
    res = write(cur_term->Filedes, mybuf, onsize + len + offsize);

    /*
     * By giving up our scheduler slot here we increase the odds that the
     * kernel will ship the contiguous clist items from the last write
     * immediately.
     */
    (void) sleep(0);

    free(mybuf);
    return (res);
}