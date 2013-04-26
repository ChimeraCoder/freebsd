
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>
#include <sys/cons.h>
#include <sys/kbio.h>
#include <sys/consio.h>
#include <sys/sysctl.h>

#include <dev/syscons/syscons.h>

static sc_softc_t sc_softcs[8];

int
sc_get_cons_priority(int *unit, int *flags)
{

	*unit = 0;
	*flags = 0;
	return (CN_INTERNAL);
}

int
sc_max_unit(void)
{
	return (1);
}

sc_softc_t *
sc_get_softc(int unit, int flags)
{
	sc_softc_t *sc;

	if (unit < 0)
		return (NULL);
	sc = &sc_softcs[unit];
	sc->unit = unit;
	if ((sc->flags & SC_INIT_DONE) == 0) {
		sc->keyboard = -1;
		sc->adapter = -1;
		sc->cursor_char = SC_CURSOR_CHAR;
		sc->mouse_char = SC_MOUSE_CHAR;
	}
	return (sc);
}

void
sc_get_bios_values(bios_values_t *values)
{
	values->cursor_start = 0;
	values->cursor_end = 32;
	values->shift_state = 0;
}

int
sc_tone(int hz)
{
	return (0);
}