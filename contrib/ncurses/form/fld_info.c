
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

MODULE_ID("$Id: fld_info.c,v 1.10 2004/12/11 22:24:57 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int field_info(const FIELD *field,
|                                   int *rows, int *cols,
|                                   int *frow, int *fcol,
|                                   int *nrow, int *nbuf)
|   
|   Description   :  Retrieve infos about the fields creation parameters.
|
|   Return Values :  E_OK           - success
|                    E_BAD_ARGUMENT - invalid field pointer
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
field_info(const FIELD *field,
	   int *rows, int *cols,
	   int *frow, int *fcol,
	   int *nrow, int *nbuf)
{
  T((T_CALLED("field_info(%p,%p,%p,%p,%p,%p,%p)"),
     field,
     rows, cols,
     frow, fcol,
     nrow, nbuf));

  if (!field)
    RETURN(E_BAD_ARGUMENT);

  if (rows)
    *rows = field->rows;
  if (cols)
    *cols = field->cols;
  if (frow)
    *frow = field->frow;
  if (fcol)
    *fcol = field->fcol;
  if (nrow)
    *nrow = field->nrow;
  if (nbuf)
    *nbuf = field->nbuf;
  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int dynamic_field_info(const FIELD *field,
|                                           int *drows, int *dcols,
|                                           int *maxgrow)
|   
|   Description   :  Retrieve informations about a dynamic fields current
|                    dynamic parameters.
|
|   Return Values :  E_OK           - success
|                    E_BAD_ARGUMENT - invalid argument
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
dynamic_field_info(const FIELD *field, int *drows, int *dcols, int *maxgrow)
{
  T((T_CALLED("dynamic_field_info(%p,%p,%p,%p)"), field, drows, dcols, maxgrow));

  if (!field)
    RETURN(E_BAD_ARGUMENT);

  if (drows)
    *drows = field->drows;
  if (dcols)
    *dcols = field->dcols;
  if (maxgrow)
    *maxgrow = field->maxgrow;

  RETURN(E_OK);
}

/* fld_info.c ends here */