
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
* Module m_item_cur                                                        *
* Set and get current menus item                                           *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_item_cur.c,v 1.17 2004/12/25 21:57:38 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_current_item(MENU *menu, const ITEM *item)
|   
|   Description   :  Make the item the current item
|
|   Return Values :  E_OK                - success
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_current_item(MENU * menu, ITEM * item)
{
  T((T_CALLED("set_current_item(%p,%p)"), menu, item));

  if (menu && item && (item->imenu == menu))
    {
      if (menu->status & _IN_DRIVER)
	RETURN(E_BAD_STATE);

      assert(menu->curitem);
      if (item != menu->curitem)
	{
	  if (menu->status & _LINK_NEEDED)
	    {
	      /*
	       * Items are available, but they are not linked together.
	       * So we have to link here.
	       */
	      _nc_Link_Items(menu);
	    }
	  assert(menu->pattern);
	  Reset_Pattern(menu);
	  /* adjust the window to make item visible and update the menu */
	  Adjust_Current_Item(menu, menu->toprow, item);
	}
    }
  else
    RETURN(E_BAD_ARGUMENT);

  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  ITEM *current_item(const MENU *menu)
|   
|   Description   :  Return the menus current item
|
|   Return Values :  Item pointer or NULL if failure
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(ITEM *)
current_item(const MENU * menu)
{
  T((T_CALLED("current_item(%p)"), menu));
  returnItem((menu && menu->items) ? menu->curitem : (ITEM *) 0);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int item_index(const ITEM *)
|   
|   Description   :  Return the logical index of this item.
|
|   Return Values :  The index or ERR if this is an invalid item pointer
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
item_index(const ITEM * item)
{
  T((T_CALLED("item_index(%p)"), item));
  returnCode((item && item->imenu) ? item->index : ERR);
}

/* m_item_cur.c ends here */