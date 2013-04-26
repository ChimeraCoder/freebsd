
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
* Module m_attribs                                                         *
* Control menus display attributes                                         *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_attribs.c,v 1.14 2004/12/11 23:29:12 tom Exp $")

/* Macro to redraw menu if it is posted and changed */
#define Refresh_Menu(menu) \
   if ( (menu) && ((menu)->status & _POSTED) )\
   {\
      _nc_Draw_Menu( menu );\
      _nc_Show_Menu( menu );\
   }

/* "Template" macro to generate a function to set a menus attribute */
#define GEN_MENU_ATTR_SET_FCT( name ) \
NCURSES_IMPEXP int NCURSES_API set_menu_ ## name (MENU * menu, chtype attr)\
{\
   T((T_CALLED("set_menu_" #name "(%p,%s)"), menu, _traceattr(attr)));\
   if (!(attr==A_NORMAL || (attr & A_ATTRIBUTES)==attr))\
      RETURN(E_BAD_ARGUMENT);\
   if (menu && ( menu -> name != attr))\
     {\
       (menu -> name) = attr;\
       Refresh_Menu(menu);\
     }\
   Normalize_Menu( menu ) -> name = attr;\
   RETURN(E_OK);\
}

/* "Template" macro to generate a function to get a menu's attribute */
#define GEN_MENU_ATTR_GET_FCT( name ) \
NCURSES_IMPEXP chtype NCURSES_API menu_ ## name (const MENU * menu)\
{\
   T((T_CALLED("menu_" #name "(%p)"), menu));\
   returnAttr(Normalize_Menu( menu ) -> name);\
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_menu_fore(MENU *menu, chtype attr)
|   
|   Description   :  Set the attribute for selectable items. In single-
|                    valued menus this is used to highlight the current
|                    item ((i.e. where the cursor is), in multi-valued
|                    menus this is used to highlight the selected items.
|
|   Return Values :  E_OK              - success
|                    E_BAD_ARGUMENT    - an invalid value has been passed   
+--------------------------------------------------------------------------*/
GEN_MENU_ATTR_SET_FCT(fore)

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  chtype menu_fore(const MENU* menu)
|   
|   Description   :  Return the attribute used for selectable items that
|                    are current (single-valued menu) or selected (multi-
|                    valued menu).   
|
|   Return Values :  Attribute value
+--------------------------------------------------------------------------*/
GEN_MENU_ATTR_GET_FCT(fore)

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_menu_back(MENU *menu, chtype attr)
|   
|   Description   :  Set the attribute for selectable but not yet selected
|                    items.
|
|   Return Values :  E_OK             - success  
|                    E_BAD_ARGUMENT   - an invalid value has been passed
+--------------------------------------------------------------------------*/
GEN_MENU_ATTR_SET_FCT(back)

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  chtype menu_back(const MENU *menu)
|   
|   Description   :  Return the attribute used for selectable but not yet
|                    selected items. 
|
|   Return Values :  Attribute value
+--------------------------------------------------------------------------*/
GEN_MENU_ATTR_GET_FCT(back)

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_menu_grey(MENU *menu, chtype attr)
|   
|   Description   :  Set the attribute for unselectable items.
|
|   Return Values :  E_OK             - success
|                    E_BAD_ARGUMENT   - an invalid value has been passed    
+--------------------------------------------------------------------------*/
GEN_MENU_ATTR_SET_FCT(grey)

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  chtype menu_grey(const MENU *menu)
|   
|   Description   :  Return the attribute used for non-selectable items
|
|   Return Values :  Attribute value
+--------------------------------------------------------------------------*/
GEN_MENU_ATTR_GET_FCT(grey)
/* m_attribs.c ends here */