
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

MODULE_ID("$Id: fld_dup.c,v 1.12 2007/10/13 19:30:21 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  FIELD *dup_field(FIELD *field, int frow, int fcol)
|
|   Description   :  Duplicates the field at the specified position. All
|                    field attributes and the buffers are copied.
|                    If an error occurs, errno is set to
|
|                    E_BAD_ARGUMENT - invalid argument
|                    E_SYSTEM_ERROR - system error
|
|   Return Values :  Pointer to the new field or NULL if failure
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(FIELD *)
dup_field(FIELD *field, int frow, int fcol)
{
  FIELD *New_Field = (FIELD *)0;
  int err = E_BAD_ARGUMENT;

  T((T_CALLED("dup_field(%p,%d,%d)"), field, frow, fcol));
  if (field && (frow >= 0) && (fcol >= 0) &&
      ((err = E_SYSTEM_ERROR) != 0) &&	/* trick : this resets the default error */
      (New_Field = typeMalloc(FIELD, 1)))
    {
      T((T_CREATE("field %p"), New_Field));
      *New_Field = *_nc_Default_Field;
      New_Field->frow = frow;
      New_Field->fcol = fcol;
      New_Field->link = New_Field;
      New_Field->rows = field->rows;
      New_Field->cols = field->cols;
      New_Field->nrow = field->nrow;
      New_Field->drows = field->drows;
      New_Field->dcols = field->dcols;
      New_Field->maxgrow = field->maxgrow;
      New_Field->nbuf = field->nbuf;
      New_Field->just = field->just;
      New_Field->fore = field->fore;
      New_Field->back = field->back;
      New_Field->pad = field->pad;
      New_Field->opts = field->opts;
      New_Field->usrptr = field->usrptr;

      if (_nc_Copy_Type(New_Field, field))
	{
	  size_t i, len;

	  len = Total_Buffer_Size(New_Field);
	  if ((New_Field->buf = (FIELD_CELL *)malloc(len)))
	    {
	      for (i = 0; i < len; ++i)
		New_Field->buf[i] = field->buf[i];
	      returnField(New_Field);
	    }
	}
    }

  if (New_Field)
    free_field(New_Field);

  SET_ERROR(err);
  returnField((FIELD *)0);
}

/* fld_dup.c ends here */