
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

#include "form.priv.h"

MODULE_ID("$Id: fld_arg.c,v 1.11 2004/12/25 22:20:18 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  int set_fieldtype_arg(
|                            FIELDTYPE *typ,
|                            void * (* const make_arg)(va_list *),
|                            void * (* const copy_arg)(const void *),
|                            void   (* const free_arg)(void *) )
|
|   Description   :  Connects to the type additional arguments necessary
|                    for a set_field_type call. The various function pointer
|                    arguments are:
|                       make_arg : allocates a structure for the field
|                                  specific parameters.
|                       copy_arg : duplicate the structure created by
|                                  make_arg
|                       free_arg : Release the memory allocated by make_arg
|                                  or copy_arg
|
|                    At least make_arg must be non-NULL.
|                    You may pass NULL for copy_arg and free_arg if your
|                    make_arg function doesn't allocate memory and your
|                    arg fits into the storage for a (void*).
|
|   Return Values :  E_OK           - success
|                    E_BAD_ARGUMENT - invalid argument
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_fieldtype_arg(FIELDTYPE *typ,
		  void *(*const make_arg)(va_list *),
		  void *(*const copy_arg)(const void *),
		  void (*const free_arg) (void *))
{
  T((T_CALLED("set_fieldtype_arg(%p,%p,%p,%p)"),
     typ, make_arg, copy_arg, free_arg));

  if (typ != 0 && make_arg != (void *)0)
    {
      typ->status |= _HAS_ARGS;
      typ->makearg = make_arg;
      typ->copyarg = copy_arg;
      typ->freearg = free_arg;
      RETURN(E_OK);
    }
  RETURN(E_BAD_ARGUMENT);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  void *field_arg(const FIELD *field)
|
|   Description   :  Retrieve pointer to the fields argument structure.
|
|   Return Values :  Pointer to structure or NULL if none is defined.
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(void *)
field_arg(const FIELD *field)
{
  T((T_CALLED("field_arg(%p)"), field));
  returnVoidPtr(Normalize_Field(field)->arg);
}

/* fld_arg.c ends here */