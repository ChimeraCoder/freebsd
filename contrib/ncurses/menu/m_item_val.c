
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
* Module m_item_val                                                        *
* Set and get menus item values                                            *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_item_val.c,v 1.14 2004/12/11 23:29:34 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_item_value(ITEM *item, int value)
|   
|   Description   :  Programmatically set the item's selection value. This is
|                    only allowed if the item is selectable at all and if
|                    it is not connected to a single-valued menu.
|                    If the item is connected to a posted menu, the menu
|                    will be redisplayed.  
|
|   Return Values :  E_OK              - success
|                    E_REQUEST_DENIED  - not selectable or single valued menu
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_item_value(ITEM * item, bool value)
{
  MENU *menu;

  T((T_CALLED("set_item_value(%p,%d)"), item, value));
  if (item)
    {
      menu = item->imenu;

      if ((!(item->opt & O_SELECTABLE)) ||
	  (menu && (menu->opt & O_ONEVALUE)))
	RETURN(E_REQUEST_DENIED);

      if (item->value ^ value)
	{
	  item->value = value ? TRUE : FALSE;
	  if (menu)
	    {
	      if (menu->status & _POSTED)
		{
		  Move_And_Post_Item(menu, item);
		  _nc_Show_Menu(menu);
		}
	    }
	}
    }
  else
    _nc_Default_Item.value = value;

  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  bool item_value(const ITEM *item)
|   
|   Description   :  Return the selection value of the item
|
|   Return Values :  TRUE   - if item is selected
|                    FALSE  - if item is not selected
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(bool)
item_value(const ITEM * item)
{
  T((T_CALLED("item_value(%p)"), item));
  returnBool((Normalize_Item(item)->value) ? TRUE : FALSE);
}

/* m_item_val.c ends here */