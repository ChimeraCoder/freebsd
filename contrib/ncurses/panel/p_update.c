
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

/* p_update.c
 * wnoutrefresh windows in an orderly fashion
 */
#include "panel.priv.h"

MODULE_ID("$Id: p_update.c,v 1.9 2005/02/19 16:49:47 tom Exp $")

NCURSES_EXPORT(void)
update_panels(void)
{
  PANEL *pan;

  T((T_CALLED("update_panels()")));
  dBug(("--> update_panels"));
  pan = _nc_bottom_panel;
  while (pan && pan->above)
    {
      PANEL_UPDATE(pan, pan->above);
      pan = pan->above;
    }

  pan = _nc_bottom_panel;
  while (pan)
    {
      Wnoutrefresh(pan);
      pan = pan->above;
    }

  returnVoid;
}