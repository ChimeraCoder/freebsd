
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
 * @brief This file contains the base phy method implementations and
 *        any constants or structures private to the base phy object.
 */

#include <dev/isci/scil/sci_base_phy.h>
#include <dev/isci/scil/sci_base_state_machine.h>

//******************************************************************************
//* P R O T E C T E D   M E T H O D S
//******************************************************************************

void sci_base_phy_construct(
   SCI_BASE_PHY_T    *this_phy,
   SCI_BASE_LOGGER_T *logger,
   SCI_BASE_STATE_T  *state_table
)
{
   sci_base_object_construct(&this_phy->parent, logger);

   sci_base_state_machine_construct(
      &this_phy->state_machine,
      &this_phy->parent,
      state_table,
      SCI_BASE_PHY_STATE_INITIAL
   );

   sci_base_state_machine_start(
      &this_phy->state_machine
   );
}