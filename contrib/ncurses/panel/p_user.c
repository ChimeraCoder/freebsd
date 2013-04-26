
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
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1995                    *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 ****************************************************************************/

/* p_user.c
 * Set/Get panels user pointer 
 */
#include "panel.priv.h"

MODULE_ID("$Id: p_user.c,v 1.6 2005/02/19 16:52:44 tom Exp $")

NCURSES_EXPORT(int)
set_panel_userptr(PANEL * pan, NCURSES_CONST void *uptr)
{
  T((T_CALLED("set_panel_userptr(%p,%p)"), pan, uptr));
  if (!pan)
    returnCode(ERR);
  pan->user = uptr;
  returnCode(OK);
}

NCURSES_EXPORT(NCURSES_CONST void *)
panel_userptr(const PANEL * pan)
{
  T((T_CALLED("panel_userptr(%p)"), pan));
  returnCVoidPtr(pan ? pan->user : (NCURSES_CONST void *)0);
}