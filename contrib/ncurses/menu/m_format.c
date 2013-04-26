
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
* Module m_format                                                          *
* Set and get maximum numbers of rows and columns in menus                 *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_format.c,v 1.15 2004/12/11 23:11:21 tom Exp $")

#define minimum(a,b) ((a)<(b) ? (a): (b))

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu
|   Function      :  int set_menu_format(MENU *menu, int rows, int cols)
|
|   Description   :  Sets the maximum number of rows and columns of items
|                    that may be displayed at one time on a menu. If the
|                    menu contains more items than can be displayed at
|                    once, the menu will be scrollable.
|
|   Return Values :  E_OK                   - success
|                    E_BAD_ARGUMENT         - invalid values passed
|                    E_NOT_CONNECTED        - there are no items connected
|                    E_POSTED               - the menu is already posted
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_menu_format(MENU * menu, int rows, int cols)
{
  int total_rows, total_cols;

  T((T_CALLED("set_menu_format(%p,%d,%d)"), menu, rows, cols));

  if (rows < 0 || cols < 0)
    RETURN(E_BAD_ARGUMENT);

  if (menu)
    {
      if (menu->status & _POSTED)
	RETURN(E_POSTED);

      if (!(menu->items))
	RETURN(E_NOT_CONNECTED);

      if (rows == 0)
	rows = menu->frows;
      if (cols == 0)
	cols = menu->fcols;

      if (menu->pattern)
	Reset_Pattern(menu);

      menu->frows = rows;
      menu->fcols = cols;

      assert(rows > 0 && cols > 0);
      total_rows = (menu->nitems - 1) / cols + 1;
      total_cols = (menu->opt & O_ROWMAJOR) ?
	minimum(menu->nitems, cols) :
	(menu->nitems - 1) / total_rows + 1;

      menu->rows = total_rows;
      menu->cols = total_cols;
      menu->arows = minimum(total_rows, rows);
      menu->toprow = 0;
      menu->curitem = *(menu->items);
      assert(menu->curitem);
      menu->status |= _LINK_NEEDED;
      _nc_Calculate_Item_Length_and_Width(menu);
    }
  else
    {
      if (rows > 0)
	_nc_Default_Menu.frows = rows;
      if (cols > 0)
	_nc_Default_Menu.fcols = cols;
    }

  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu
|   Function      :  void menu_format(const MENU *menu, int *rows, int *cols)
|
|   Description   :  Returns the maximum number of rows and columns that may
|                    be displayed at one time on menu.
|
|   Return Values :  -
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(void)
menu_format(const MENU * menu, int *rows, int *cols)
{
  if (rows)
    *rows = Normalize_Menu(menu)->frows;
  if (cols)
    *cols = Normalize_Menu(menu)->fcols;
}

/* m_format.c ends here */