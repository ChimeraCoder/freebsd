
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
* Module m_userptr                                                         *
* Associate application data with menus                                    *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_userptr.c,v 1.16 2004/12/25 21:38:55 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_menu_userptr(MENU *menu, void *userptr)
|   
|   Description   :  Set the pointer that is reserved in any menu to store
|                    application relevant informations.
|
|   Return Values :  E_OK         - success
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_menu_userptr(MENU * menu, void *userptr)
{
  T((T_CALLED("set_menu_userptr(%p,%p)"), menu, userptr));
  Normalize_Menu(menu)->userptr = userptr;
  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  void *menu_userptr(const MENU *menu)
|   
|   Description   :  Return the pointer that is reserved in any menu to
|                    store application relevant informations.
|
|   Return Values :  Value of the pointer. If no such pointer has been set,
|                    NULL is returned
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(void *)
menu_userptr(const MENU * menu)
{
  T((T_CALLED("menu_userptr(%p)"), menu));
  returnVoidPtr(Normalize_Menu(menu)->userptr);
}

/* m_userptr.c ends here */