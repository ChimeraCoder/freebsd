
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

/* p_delete.c
 * Remove a panel from stack, if in it, and free struct
 */
#include "panel.priv.h"

MODULE_ID("$Id: p_delete.c,v 1.8 2005/02/19 16:38:45 tom Exp $")

NCURSES_EXPORT(int)
del_panel(PANEL * pan)
{
  int err = OK;

  T((T_CALLED("del_panel(%p)"), pan));
  if (pan)
    {
      dBug(("--> del_panel %s", USER_PTR(pan->user)));
      HIDE_PANEL(pan, err, OK);
      free((void *)pan);
    }
  else
    err = ERR;

  returnCode(err);
}