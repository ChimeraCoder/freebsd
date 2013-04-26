
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
 *   Author:  Juergen Pfeifer, 1995,1997                                    *
 ****************************************************************************/

/***************************************************************************
* Module m_item_vis                                                        *
* Tell if menu item is visible                                             *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_item_vis.c,v 1.15 2004/12/25 21:40:09 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  bool item_visible(const ITEM *item)
|   
|   Description   :  A item is visible if it currently appears in the
|                    subwindow of a posted menu.
|
|   Return Values :  TRUE  if visible
|                    FALSE if invisible
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(bool)
item_visible(const ITEM * item)
{
  MENU *menu;

  T((T_CALLED("item_visible(%p)"), item));
  if (item &&
      (menu = item->imenu) &&
      (menu->status & _POSTED) &&
      ((menu->toprow + menu->arows) > (item->y)) &&
      (item->y >= menu->toprow))
    returnBool(TRUE);
  else
    returnBool(FALSE);
}

/* m_item_vis.c ends here */