
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
* Module m_sub                                                             *
* Menus subwindow association routines                                     *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_sub.c,v 1.10 2004/12/25 21:38:31 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_menu_sub(MENU *menu, WINDOW *win)
|   
|   Description   :  Sets the subwindow of the menu.
|
|   Return Values :  E_OK           - success
|                    E_POSTED       - menu is already posted
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_menu_sub(MENU * menu, WINDOW *win)
{
  T((T_CALLED("set_menu_sub(%p,%p)"), menu, win));

  if (menu)
    {
      if (menu->status & _POSTED)
	RETURN(E_POSTED);
      menu->usersub = win;
      _nc_Calculate_Item_Length_and_Width(menu);
    }
  else
    _nc_Default_Menu.usersub = win;

  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  WINDOW *menu_sub(const MENU *menu)
|   
|   Description   :  Returns a pointer to the subwindow of the menu
|
|   Return Values :  NULL on error, otherwise a pointer to the window
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(WINDOW *)
menu_sub(const MENU * menu)
{
  const MENU *m = Normalize_Menu(menu);

  T((T_CALLED("menu_sub(%p)"), menu));
  returnWin(Get_Menu_Window(m));
}

/* m_sub.c ends here */