
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

#include <sys/param.h>
#include <sys/bus.h>

#include <contrib/dev/acpica/include/acpi.h>

#include <dev/acpica/acpivar.h>

int
acpi_sleep_machdep(struct acpi_softc *sc, int state)
{
	return (0);
}

int
acpi_wakeup_machdep(struct acpi_softc *sc, int state, int sleep_result,
    int intr_enabled)
{
	return (0);
}

void
acpi_install_wakeup_handler(struct acpi_softc *sc)
{
}