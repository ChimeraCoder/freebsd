
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
* Module m_pad                                                             *
* Control menus padding character                                          *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_pad.c,v 1.10 2004/12/25 21:40:58 tom Exp $")

/* Macro to redraw menu if it is posted and changed */
#define Refresh_Menu(menu) \
   if ( (menu) && ((menu)->status & _POSTED) )\
   {\
      _nc_Draw_Menu( menu );\
      _nc_Show_Menu( menu );\
   }

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_menu_pad(MENU *menu, int pad)
|   
|   Description   :  Set the character to be used to separate the item name
|                    from its description. This must be a printable 
|                    character.
|
|   Return Values :  E_OK              - success
|                    E_BAD_ARGUMENT    - an invalid value has been passed
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_menu_pad(MENU * menu, int pad)
{
  bool do_refresh = (menu != (MENU *) 0);

  T((T_CALLED("set_menu_pad(%p,%d)"), menu, pad));

  if (!isprint(UChar(pad)))
    RETURN(E_BAD_ARGUMENT);

  Normalize_Menu(menu);
  menu->pad = pad;

  if (do_refresh)
    Refresh_Menu(menu);

  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int menu_pad(const MENU *menu)
|   
|   Description   :  Return the value of the padding character
|
|   Return Values :  The pad character
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
menu_pad(const MENU * menu)
{
  T((T_CALLED("menu_pad(%p)"), menu));
  returnCode(Normalize_Menu(menu)->pad);
}

/* m_pad.c ends here */