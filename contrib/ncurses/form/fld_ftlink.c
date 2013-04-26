
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

MODULE_ID("$Id: fld_ftlink.c,v 1.13 2007/10/13 19:30:35 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  FIELDTYPE *link_fieldtype(
|                                FIELDTYPE *type1,
|                                FIELDTYPE *type2)
|   
|   Description   :  Create a new fieldtype built from the two given types.
|                    They are connected by an logical 'OR'.
|                    If an error occurs, errno is set to                    
|                       E_BAD_ARGUMENT  - invalid arguments
|                       E_SYSTEM_ERROR  - system error (no memory)
|
|   Return Values :  Fieldtype pointer or NULL if error occurred.
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(FIELDTYPE *)
link_fieldtype(FIELDTYPE *type1, FIELDTYPE *type2)
{
  FIELDTYPE *nftyp = (FIELDTYPE *)0;

  T((T_CALLED("link_fieldtype(%p,%p)"), type1, type2));
  if (type1 && type2)
    {
      nftyp = typeMalloc(FIELDTYPE, 1);

      if (nftyp)
	{
	  T((T_CREATE("fieldtype %p"), nftyp));
	  *nftyp = *_nc_Default_FieldType;
	  nftyp->status |= _LINKED_TYPE;
	  if ((type1->status & _HAS_ARGS) || (type2->status & _HAS_ARGS))
	    nftyp->status |= _HAS_ARGS;
	  if ((type1->status & _HAS_CHOICE) || (type2->status & _HAS_CHOICE))
	    nftyp->status |= _HAS_CHOICE;
	  nftyp->left = type1;
	  nftyp->right = type2;
	  type1->ref++;
	  type2->ref++;
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

/* fld_ftlink.c ends here */