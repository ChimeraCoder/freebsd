
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
* Module m_item_top                                                        *
* Set and get top menus item                                               *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_item_top.c,v 1.10 2004/12/11 23:29:34 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_top_row(MENU *menu, int row)
|   
|   Description   :  Makes the specified row the top row in the menu
|
|   Return Values :  E_OK             - success
|                    E_BAD_ARGUMENT   - not a menu pointer or invalid row
|                    E_NOT_CONNECTED  - there are no items for the menu
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_top_row(MENU * menu, int row)
{
  ITEM *item;

  T((T_CALLED("set_top_row(%p,%d)"), menu, row));

  if (menu)
    {
      if (menu->status & _IN_DRIVER)
	RETURN(E_BAD_STATE);
      if (menu->items == (ITEM **) 0)
	RETURN(E_NOT_CONNECTED);

      if ((row < 0) || (row > (menu->rows - menu->arows)))
	RETURN(E_BAD_ARGUMENT);
    }
  else
    RETURN(E_BAD_ARGUMENT);

  if (row != menu->toprow)
    {
      if (menu->status & _LINK_NEEDED)
	_nc_Link_Items(menu);

      item = menu->items[(menu->opt & O_ROWMAJOR) ? (row * menu->cols) : row];
      assert(menu->pattern);
      Reset_Pattern(menu);
      _nc_New_TopRow_and_CurrentItem(menu, row, item);
    }

  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int top_row(const MENU *)
|   
|   Description   :  Return the top row of the menu
|
|   Return Values :  The row number or ERR if there is no row
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
top_row(const MENU * menu)
{
  T((T_CALLED("top_row(%p)"), menu));
  if (menu && menu->items && *(menu->items))
    {
      assert((menu->toprow >= 0) && (menu->toprow < menu->rows));
      returnCode(menu->toprow);
    }
  else
    returnCode(ERR);
}

/* m_item_top.c ends here */