
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

/* p_bottom.c
 * Place a panel on bottom of the stack; may already be in the stack 
 */
#include "panel.priv.h"

MODULE_ID("$Id: p_bottom.c,v 1.11 2005/02/19 16:38:16 tom Exp $")

NCURSES_EXPORT(int)
bottom_panel(PANEL * pan)
{
  int err = OK;

  T((T_CALLED("bottom_panel(%p)"), pan));
  if (pan)
    {
      if (!Is_Bottom(pan))
	{

	  dBug(("--> bottom_panel %s", USER_PTR(pan->user)));

	  HIDE_PANEL(pan, err, OK);
	  assert(_nc_bottom_panel == _nc_stdscr_pseudo_panel);

	  dStack("<lb%d>", 1, pan);

	  pan->below = _nc_bottom_panel;
	  pan->above = _nc_bottom_panel->above;
	  if (pan->above)
	    pan->above->below = pan;
	  _nc_bottom_panel->above = pan;

	  dStack("<lb%d>", 9, pan);
	}
    }
  else
    err = ERR;

  returnCode(err);
}