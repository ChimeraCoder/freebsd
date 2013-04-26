
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
* Module m_new                                                             *
* Creation and destruction of new menus                                    *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_new.c,v 1.18 2006/11/04 19:04:06 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  MENU *new_menu(ITEM **items)
|   
|   Description   :  Creates a new menu connected to the item pointer
|                    array items and returns a pointer to the new menu.
|                    The new menu is initialized with the values from the
|                    default menu.
|
|   Return Values :  NULL on error
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(MENU *)
new_menu(ITEM ** items)
{
  int err = E_SYSTEM_ERROR;
  MENU *menu = (MENU *) calloc(1, sizeof(MENU));

  T((T_CALLED("new_menu(%p)"), items));
  if (menu)
    {
      *menu = _nc_Default_Menu;
      menu->status = 0;
      menu->rows = menu->frows;
      menu->cols = menu->fcols;
      if (items && *items)
	{
	  if (!_nc_Connect_Items(menu, items))
	    {
	      err = E_NOT_CONNECTED;
	      free(menu);
	      menu = (MENU *) 0;
	    }
	}
    }

  if (!menu)
    SET_ERROR(err);

  returnMenu(menu);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int free_menu(MENU *menu)  
|   
|   Description   :  Disconnects menu from its associated item pointer 
|                    array and frees the storage allocated for the menu.
|
|   Return Values :  E_OK               - success
|                    E_BAD_ARGUMENT     - Invalid menu pointer passed
|                    E_POSTED           - Menu is already posted
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
free_menu(MENU * menu)
{
  T((T_CALLED("free_menu(%p)"), menu));
  if (!menu)
    RETURN(E_BAD_ARGUMENT);

  if (menu->status & _POSTED)
    RETURN(E_POSTED);

  if (menu->items)
    _nc_Disconnect_Items(menu);

  if ((menu->status & _MARK_ALLOCATED) && menu->mark)
    free(menu->mark);

  free(menu);
  RETURN(E_OK);
}

/* m_new.c ends here */