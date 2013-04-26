
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
* Module m_cursor                                                          *
* Correctly position a menu's cursor                                       *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_cursor.c,v 1.20 2005/10/22 23:03:32 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu
|   Function      :  _nc_menu_cursor_pos
|
|   Description   :  Return position of logical cursor to current item
|
|   Return Values :  E_OK            - success
|                    E_BAD_ARGUMENT  - invalid menu
|                    E_NOT_POSTED    - Menu is not posted
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
_nc_menu_cursor_pos(const MENU * menu, const ITEM * item, int *pY, int *pX)
{
  if (!menu || !pX || !pY)
    return (E_BAD_ARGUMENT);
  else
    {
      if ((ITEM *) 0 == item)
	item = menu->curitem;
      assert(item != (ITEM *) 0);

      if (!(menu->status & _POSTED))
	return (E_NOT_POSTED);

      *pX = item->x * (menu->spc_cols + menu->itemlen);
      *pY = (item->y - menu->toprow) * menu->spc_rows;
    }
  return (E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu
|   Function      :  pos_menu_cursor
|
|   Description   :  Position logical cursor to current item in menu
|
|   Return Values :  E_OK            - success
|                    E_BAD_ARGUMENT  - invalid menu
|                    E_NOT_POSTED    - Menu is not posted
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
pos_menu_cursor(const MENU * menu)
{
  WINDOW *win, *sub;
  int x = 0, y = 0;
  int err = _nc_menu_cursor_pos(menu, (ITEM *) 0, &y, &x);

  T((T_CALLED("pos_menu_cursor(%p)"), menu));

  if (E_OK == err)
    {
      win = menu->userwin ? menu->userwin : stdscr;
      sub = menu->usersub ? menu->usersub : win;
      assert(win && sub);

      if ((menu->opt & O_SHOWMATCH) && (menu->pindex > 0))
	x += (menu->pindex + menu->marklen - 1);

      wmove(sub, y, x);

      if (win != sub)
	{
	  wcursyncup(sub);
	  wsyncup(sub);
	  untouchwin(sub);
	}
    }
  RETURN(err);
}

/* m_cursor.c ends here */