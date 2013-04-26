
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
 *   Author:  Thomas E. Dickey                                              *
 ****************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_trace.c,v 1.3 2005/01/16 01:06:11 tom Exp $")

NCURSES_EXPORT(ITEM *)
_nc_retrace_item(ITEM * code)
{
  T((T_RETURN("%p"), code));
  return code;
}

NCURSES_EXPORT(ITEM **)
_nc_retrace_item_ptr(ITEM ** code)
{
  T((T_RETURN("%p"), code));
  return code;
}

NCURSES_EXPORT(Item_Options)
_nc_retrace_item_opts(Item_Options code)
{
  T((T_RETURN("%d"), code));
  return code;
}

NCURSES_EXPORT(MENU *)
_nc_retrace_menu(MENU * code)
{
  T((T_RETURN("%p"), code));
  return code;
}

NCURSES_EXPORT(Menu_Hook)
_nc_retrace_menu_hook(Menu_Hook code)
{
  T((T_RETURN("%p"), code));
  return code;
}

NCURSES_EXPORT(Menu_Options)
_nc_retrace_menu_opts(Menu_Options code)
{
  T((T_RETURN("%d"), code));
  return code;
}