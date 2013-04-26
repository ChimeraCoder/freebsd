
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
 * @brief This file contains the base domain method implementations and
 *        any constants or structures private to the base domain object.
 */

#include <dev/isci/scil/sci_base_domain.h>
#include <dev/isci/scil/sci_base_state_machine.h>

//******************************************************************************
//* P R O T E C T E D   M E T H O D S
//******************************************************************************

void sci_base_domain_construct(
   SCI_BASE_DOMAIN_T * this_domain,
   SCI_BASE_LOGGER_T * logger,
   SCI_BASE_STATE_T  * state_table
)
{
   sci_base_object_construct(&this_domain->parent, logger);

   sci_base_state_machine_construct(
      &this_domain->state_machine,
      &this_domain->parent,
      state_table,
      SCI_BASE_DOMAIN_STATE_INITIAL
   );

   sci_base_state_machine_start(
      &this_domain->state_machine
   );
}