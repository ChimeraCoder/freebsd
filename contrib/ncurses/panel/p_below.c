
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

/* p_below.c
 */
#include "panel.priv.h"

MODULE_ID("$Id: p_below.c,v 1.6 2005/02/19 16:45:10 tom Exp $")

NCURSES_EXPORT(PANEL *)
panel_below(const PANEL * pan)
{
  T((T_CALLED("panel_below(%p)"), pan));
  if (!pan)
    {
      /* if top and bottom are equal, we have no or only the pseudo panel */
      returnPanel(EMPTY_STACK()? (PANEL *) 0 : _nc_top_panel);
    }
  else
    {
      /* we must not return the pseudo panel */
      returnPanel(Is_Pseudo(pan->below) ? (PANEL *) 0 : pan->below);
    }
}