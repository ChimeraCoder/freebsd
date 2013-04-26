
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

MODULE_ID("$Id: fld_newftyp.c,v 1.15 2007/10/13 19:30:55 tom Exp $")

static FIELDTYPE const default_fieldtype =
{
  0,				/* status                                      */
  0L,				/* reference count                             */
  (FIELDTYPE *)0,		/* pointer to left  operand                    */
  (FIELDTYPE *)0,		/* pointer to right operand                    */
  NULL,				/* makearg function                            */
  NULL,				/* copyarg function                            */
  NULL,				/* freearg function                            */
  NULL,				/* field validation function                   */
  NULL,				/* Character check function                    */
  NULL,				/* enumerate next function                     */
  NULL				/* enumerate previous function                 */
};

NCURSES_EXPORT_VAR(const FIELDTYPE *)
_nc_Default_FieldType = &default_fieldtype;

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  FIELDTYPE *new_fieldtype(
|                       bool (* const field_check)(FIELD *,const void *),
|                       bool (* const char_check) (int, const void *) )
|
|   Description   :  Create a new fieldtype. The application programmer must
|                    write a field_check and a char_check function and give
|                    them as input to this call.
|                    If an error occurs, errno is set to
|                       E_BAD_ARGUMENT  - invalid arguments
|                       E_SYSTEM_ERROR  - system error (no memory)
|
|   Return Values :  Fieldtype pointer or NULL if error occurred
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(FIELDTYPE *)
new_fieldtype(bool (*const field_check) (FIELD *, const void *),
	      bool (*const char_check) (int, const void *))
{
  FIELDTYPE *nftyp = (FIELDTYPE *)0;

  T((T_CALLED("new_fieldtype(%p,%p)"), field_check, char_check));
  if ((field_check) || (char_check))
    {
      nftyp = typeMalloc(FIELDTYPE, 1);

      if (nftyp)
	{
	  T((T_CREATE("fieldtype %p"), nftyp));
	  *nftyp = default_fieldtype;
	  nftyp->fcheck = field_check;
	  nftyp->ccheck = char_check;
	}
      else
	{
	  SET_ERROR(E_SYSTEM_ERROR);
	}
    }
  else
    {
      SET_ERROR(E_BAD_ARGUMENT);
    }
  returnFieldType(nftyp);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  int free_fieldtype(FIELDTYPE *typ)
|
|   Description   :  Release the memory associated with this fieldtype.
|
|   Return Values :  E_OK            - success
|                    E_CONNECTED     - there are fields referencing the type
|                    E_BAD_ARGUMENT  - invalid fieldtype pointer
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
free_fieldtype(FIELDTYPE *typ)
{
  T((T_CALLED("free_fieldtype(%p)"), typ));

  if (!typ)
    RETURN(E_BAD_ARGUMENT);

  if (typ->ref != 0)
    RETURN(E_CONNECTED);

  if (typ->status & _RESIDENT)
    RETURN(E_CONNECTED);

  if (typ->status & _LINKED_TYPE)
    {
      if (typ->left)
	typ->left->ref--;
      if (typ->right)
	typ->right->ref--;
    }
  free(typ);
  RETURN(E_OK);
}

/* fld_newftyp.c ends here */