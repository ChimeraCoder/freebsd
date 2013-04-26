
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
* Module m_item_opt                                                        *
* Menus item option routines                                               *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_item_opt.c,v 1.17 2004/12/25 21:32:54 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_item_opts(ITEM *item, Item_Options opts)  
|   
|   Description   :  Set the options of the item. If there are relevant
|                    changes, the item is connected and the menu is posted,
|                    the menu will be redisplayed.
|
|   Return Values :  E_OK            - success
|                    E_BAD_ARGUMENT  - invalid item options
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_item_opts(ITEM * item, Item_Options opts)
{
  T((T_CALLED("set_menu_opts(%p,%d)"), item, opts));

  opts &= ALL_ITEM_OPTS;

  if (opts & ~ALL_ITEM_OPTS)
    RETURN(E_BAD_ARGUMENT);

  if (item)
    {
      if (item->opt != opts)
	{
	  MENU *menu = item->imenu;

	  item->opt = opts;

	  if ((!(opts & O_SELECTABLE)) && item->value)
	    item->value = FALSE;

	  if (menu && (menu->status & _POSTED))
	    {
	      Move_And_Post_Item(menu, item);
	      _nc_Show_Menu(menu);
	    }
	}
    }
  else
    _nc_Default_Item.opt = opts;

  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int item_opts_off(ITEM *item, Item_Options opts)   
|   
|   Description   :  Switch of the options for this item.
|
|   Return Values :  E_OK            - success
|                    E_BAD_ARGUMENT  - invalid options
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
item_opts_off(ITEM * item, Item_Options opts)
{
  ITEM *citem = item;		/* use a copy because set_item_opts must detect

				   NULL item itself to adjust its behavior */

  T((T_CALLED("item_opts_off(%p,%d)"), item, opts));

  if (opts & ~ALL_ITEM_OPTS)
    RETURN(E_BAD_ARGUMENT);
  else
    {
      Normalize_Item(citem);
      opts = citem->opt & ~(opts & ALL_ITEM_OPTS);
      returnCode(set_item_opts(item, opts));
    }
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int item_opts_on(ITEM *item, Item_Options opts)   
|   
|   Description   :  Switch on the options for this item.
|
|   Return Values :  E_OK            - success
|                    E_BAD_ARGUMENT  - invalid options
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
item_opts_on(ITEM * item, Item_Options opts)
{
  ITEM *citem = item;		/* use a copy because set_item_opts must detect

				   NULL item itself to adjust its behavior */

  T((T_CALLED("item_opts_on(%p,%d)"), item, opts));

  opts &= ALL_ITEM_OPTS;
  if (opts & ~ALL_ITEM_OPTS)
    RETURN(E_BAD_ARGUMENT);
  else
    {
      Normalize_Item(citem);
      opts = citem->opt | opts;
      returnCode(set_item_opts(item, opts));
    }
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  Item_Options item_opts(const ITEM *item)   
|   
|   Description   :  Switch of the options for this item.
|
|   Return Values :  Items options
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(Item_Options)
item_opts(const ITEM * item)
{
  T((T_CALLED("item_opts(%p)"), item));
  returnItemOpts(ALL_ITEM_OPTS & Normalize_Item(item)->opt);
}

/* m_item_opt.c ends here */