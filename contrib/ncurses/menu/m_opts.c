
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
* Module m_opts                                                            *
* Menus option routines                                                    *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_opts.c,v 1.19 2004/12/25 21:36:12 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu
|   Function      :  int set_menu_opts(MENU *menu, Menu_Options opts)
|
|   Description   :  Set the options for this menu. If the new settings
|                    end up in a change of the geometry of the menu, it
|                    will be recalculated. This operation is forbidden if
|                    the menu is already posted.
|
|   Return Values :  E_OK           - success
|                    E_BAD_ARGUMENT - invalid menu options
|                    E_POSTED       - menu is already posted
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_menu_opts(MENU * menu, Menu_Options opts)
{
  T((T_CALLED("set_menu_opts(%p,%d)"), menu, opts));

  opts &= ALL_MENU_OPTS;

  if (opts & ~ALL_MENU_OPTS)
    RETURN(E_BAD_ARGUMENT);

  if (menu)
    {
      if (menu->status & _POSTED)
	RETURN(E_POSTED);

      if ((opts & O_ROWMAJOR) != (menu->opt & O_ROWMAJOR))
	{
	  /* we need this only if the layout really changed ... */
	  if (menu->items && menu->items[0])
	    {
	      menu->toprow = 0;
	      menu->curitem = menu->items[0];
	      assert(menu->curitem);
	      set_menu_format(menu, menu->frows, menu->fcols);
	    }
	}

      menu->opt = opts;

      if (opts & O_ONEVALUE)
	{
	  ITEM **item;

	  if (((item = menu->items) != (ITEM **) 0))
	    for (; *item; item++)
	      (*item)->value = FALSE;
	}

      if (opts & O_SHOWDESC)	/* this also changes the geometry */
	_nc_Calculate_Item_Length_and_Width(menu);
    }
  else
    _nc_Default_Menu.opt = opts;

  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu
|   Function      :  int menu_opts_off(MENU *menu, Menu_Options opts)
|
|   Description   :  Switch off the options for this menu. If the new settings
|                    end up in a change of the geometry of the menu, it
|                    will be recalculated. This operation is forbidden if
|                    the menu is already posted.
|
|   Return Values :  E_OK           - success
|                    E_BAD_ARGUMENT - invalid options
|                    E_POSTED       - menu is already posted
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
menu_opts_off(MENU * menu, Menu_Options opts)
{
  MENU *cmenu = menu;		/* use a copy because set_menu_opts must detect

				   NULL menu itself to adjust its behavior */

  T((T_CALLED("menu_opts_off(%p,%d)"), menu, opts));

  opts &= ALL_MENU_OPTS;
  if (opts & ~ALL_MENU_OPTS)
    RETURN(E_BAD_ARGUMENT);
  else
    {
      Normalize_Menu(cmenu);
      opts = cmenu->opt & ~opts;
      returnCode(set_menu_opts(menu, opts));
    }
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu
|   Function      :  int menu_opts_on(MENU *menu, Menu_Options opts)
|
|   Description   :  Switch on the options for this menu. If the new settings
|                    end up in a change of the geometry of the menu, it
|                    will be recalculated. This operation is forbidden if
|                    the menu is already posted.
|
|   Return Values :  E_OK           - success
|                    E_BAD_ARGUMENT - invalid menu options
|                    E_POSTED       - menu is already posted
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
menu_opts_on(MENU * menu, Menu_Options opts)
{
  MENU *cmenu = menu;		/* use a copy because set_menu_opts must detect

				   NULL menu itself to adjust its behavior */

  T((T_CALLED("menu_opts_on(%p,%d)"), menu, opts));

  opts &= ALL_MENU_OPTS;
  if (opts & ~ALL_MENU_OPTS)
    RETURN(E_BAD_ARGUMENT);
  else
    {
      Normalize_Menu(cmenu);
      opts = cmenu->opt | opts;
      returnCode(set_menu_opts(menu, opts));
    }
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu
|   Function      :  Menu_Options menu_opts(const MENU *menu)
|
|   Description   :  Return the options for this menu.
|
|   Return Values :  Menu options
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(Menu_Options)
menu_opts(const MENU * menu)
{
  T((T_CALLED("menu_opts(%p)"), menu));
  returnMenuOpts(ALL_MENU_OPTS & Normalize_Menu(menu)->opt);
}

/* m_opts.c ends here */