
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/**
 * @file
 *
 * @brief This file contains all of the method imlementations for the
 *        SCI_BASE_LIBRARY object.
 */

#include <dev/isci/scil/sci_base_library.h>

//******************************************************************************
//* P U B L I C   M E T H O D S
//******************************************************************************

U32 sci_library_get_major_version(
   void
)
{
   // Return the 32-bit value representing the major version for this SCI
   // binary.
   return __SCI_LIBRARY_MAJOR_VERSION__;
}

// ---------------------------------------------------------------------------

U32 sci_library_get_minor_version(
   void
)
{
   // Return the 32-bit value representing the minor version for this SCI
   // binary.
   return __SCI_LIBRARY_MINOR_VERSION__;
}

// ---------------------------------------------------------------------------

U32 sci_library_get_build_version(
   void
)
{
   // Return the 32-bit value representing the build version for this SCI
   // binary.
   return __SCI_LIBRARY_BUILD_VERSION__;
}

//******************************************************************************
//* P R O T E C T E D   M E T H O D S
//******************************************************************************