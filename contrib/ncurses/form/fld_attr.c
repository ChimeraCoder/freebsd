
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

MODULE_ID("$Id: fld_attr.c,v 1.10 2004/12/11 21:33:15 tom Exp $")

/*----------------------------------------------------------------------------
  Field-Attribute manipulation routines
  --------------------------------------------------------------------------*/
/* "Template" macro to generate a function to set a fields attribute */
#define GEN_FIELD_ATTR_SET_FCT( name ) \
NCURSES_IMPEXP int NCURSES_API set_field_ ## name (FIELD * field, chtype attr)\
{\
   int res = E_BAD_ARGUMENT;\
   T((T_CALLED("set_field_" #name "(%p,%s)"), field, _traceattr(attr)));\
   if ( attr==A_NORMAL || ((attr & A_ATTRIBUTES)==attr) )\
     {\
       Normalize_Field( field );\
       if (field != 0) \
	 { \
	 if ((field -> name) != attr)\
	   {\
	     field -> name = attr;\
	     res = _nc_Synchronize_Attributes( field );\
	   }\
	 else\
	   {\
	     res = E_OK;\
	   }\
	 }\
     }\
   RETURN(res);\
}

/* "Template" macro to generate a function to get a fields attribute */
#define GEN_FIELD_ATTR_GET_FCT( name ) \
NCURSES_IMPEXP chtype NCURSES_API field_ ## name (const FIELD * field)\
{\
   T((T_CALLED("field_" #name "(%p)"), field));\
   returnAttr( A_ATTRIBUTES & (Normalize_Field( field ) -> name) );\
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  int set_field_fore(FIELD *field, chtype attr)
|
|   Description   :  Sets the foreground of the field used to display the
|                    field contents.
|
|   Return Values :  E_OK             - success
|                    E_BAD_ARGUMENT   - invalid attributes
|                    E_SYSTEM_ERROR   - system error
+--------------------------------------------------------------------------*/
GEN_FIELD_ATTR_SET_FCT(fore)

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  chtype field_fore(const FIELD *)
|
|   Description   :  Retrieve fields foreground attribute
|
|   Return Values :  The foreground attribute
+--------------------------------------------------------------------------*/
GEN_FIELD_ATTR_GET_FCT(fore)

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  int set_field_back(FIELD *field, chtype attr)
|
|   Description   :  Sets the background of the field used to display the
|                    fields extend.
|
|   Return Values :  E_OK             - success
|                    E_BAD_ARGUMENT   - invalid attributes
|                    E_SYSTEM_ERROR   - system error
+--------------------------------------------------------------------------*/
GEN_FIELD_ATTR_SET_FCT(back)

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  chtype field_back(const
|
|   Description   :  Retrieve fields background attribute
|
|   Return Values :  The background attribute
+--------------------------------------------------------------------------*/
GEN_FIELD_ATTR_GET_FCT(back)

/* fld_attr.c ends here */