
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

/* p_above.c
 */
#include "panel.priv.h"

MODULE_ID("$Id: p_above.c,v 1.6 2005/02/19 16:44:57 tom Exp $")

NCURSES_EXPORT(PANEL *)
panel_above(const PANEL * pan)
{
  T((T_CALLED("panel_above(%p)"), pan));
  if (!pan)
    {
      /* if top and bottom are equal, we have no or only the pseudo panel;
         if not, we return the panel above the pseudo panel */
      returnPanel(EMPTY_STACK()? (PANEL *) 0 : _nc_bottom_panel->above);
    }
  else
    returnPanel(pan->above);
}