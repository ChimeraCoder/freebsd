
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
* Module m_items                                                           *
* Connect and disconnect items to and from menus                           *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_items.c,v 1.16 2005/01/16 01:02:23 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_menu_items(MENU *menu, ITEM **items)
|   
|   Description   :  Sets the item pointer array connected to menu.
|
|   Return Values :  E_OK           - success
|                    E_POSTED       - menu is already posted
|                    E_CONNECTED    - one or more items are already connected
|                                     to another menu.
|                    E_BAD_ARGUMENT - An incorrect menu or item array was
|                                     passed to the function
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_menu_items(MENU * menu, ITEM ** items)
{
  T((T_CALLED("set_menu_items(%p,%p)"), menu, items));

  if (!menu || (items && !(*items)))
    RETURN(E_BAD_ARGUMENT);

  if (menu->status & _POSTED)
    RETURN(E_POSTED);

  if (menu->items)
    _nc_Disconnect_Items(menu);

  if (items)
    {
      if (!_nc_Connect_Items(menu, items))
	RETURN(E_CONNECTED);
    }

  menu->items = items;
  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  ITEM **menu_items(const MENU *menu)
|   
|   Description   :  Returns a pointer to the item pointer array of the menu
|
|   Return Values :  NULL on error
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(ITEM **)
menu_items(const MENU * menu)
{
  T((T_CALLED("menu_items(%p)"), menu));
  returnItemPtr(menu ? menu->items : (ITEM **) 0);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int item_count(const MENU *menu)
|   
|   Description   :  Get the number of items connected to the menu. If the
|                    menu pointer is NULL we return -1.         
|
|   Return Values :  Number of items or -1 to indicate error.
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
item_count(const MENU * menu)
{
  T((T_CALLED("item_count(%p)"), menu));
  returnCode(menu ? menu->nitems : -1);
}

/* m_items.c ends here */