
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

#include "form.priv.h"

MODULE_ID("$Id: f_trace.c,v 1.1 2004/12/25 23:28:49 tom Exp $")

NCURSES_EXPORT(FIELD **)
_nc_retrace_field_ptr(FIELD **code)
{
  T((T_RETURN("%p"), code));
  return code;
}

NCURSES_EXPORT(FIELD *)
_nc_retrace_field(FIELD *code)
{
  T((T_RETURN("%p"), code));
  return code;
}

NCURSES_EXPORT(FIELDTYPE *)
_nc_retrace_field_type(FIELDTYPE *code)
{
  T((T_RETURN("%p"), code));
  return code;
}

NCURSES_EXPORT(FORM *)
_nc_retrace_form(FORM *code)
{
  T((T_RETURN("%p"), code));
  return code;
}

NCURSES_EXPORT(Form_Hook)
_nc_retrace_form_hook(Form_Hook code)
{
  T((T_RETURN("%p"), code));
  return code;
}