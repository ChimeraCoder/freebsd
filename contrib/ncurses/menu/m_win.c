
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
* Module m_win                                                             *
* Menus window association routines                                        *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_win.c,v 1.15 2004/12/25 21:39:20 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_menu_win(MENU *menu, WINDOW *win)
|   
|   Description   :  Sets the window of the menu.
|
|   Return Values :  E_OK               - success
|                    E_POSTED           - menu is already posted
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_menu_win(MENU * menu, WINDOW *win)
{
  T((T_CALLED("set_menu_win(%p,%p)"), menu, win));

  if (menu)
    {
      if (menu->status & _POSTED)
	RETURN(E_POSTED);
      menu->userwin = win;
      _nc_Calculate_Item_Length_and_Width(menu);
    }
  else
    _nc_Default_Menu.userwin = win;

  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  WINDOW *menu_win(const MENU *)
|   
|   Description   :  Returns pointer to the window of the menu
|
|   Return Values :  NULL on error, otherwise pointer to window
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(WINDOW *)
menu_win(const MENU * menu)
{
  const MENU *m = Normalize_Menu(menu);

  T((T_CALLED("menu_win(%p)"), menu));
  returnWin(m->userwin ? m->userwin : stdscr);
}

/* m_win.c ends here */