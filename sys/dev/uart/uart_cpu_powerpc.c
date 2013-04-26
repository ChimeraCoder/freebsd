
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
#include <vm/vm.h>
#include <vm/pmap.h>

#include <machine/bus.h>
#include <machine/ofw_machdep.h>

#include <dev/ofw/openfirm.h>
#include <dev/uart/uart.h>
#include <dev/uart/uart_cpu.h>

bus_space_tag_t uart_bus_space_io = &bs_le_tag;
bus_space_tag_t uart_bus_space_mem = &bs_le_tag;

int
uart_cpu_eqres(struct uart_bas *b1, struct uart_bas *b2)
{

	return ((pmap_kextract(b1->bsh) == pmap_kextract(b2->bsh)) ? 1 : 0);
}

static int
ofw_get_uart_console(phandle_t opts, phandle_t *result, const char *inputdev,
    const char *outputdev)
{
	char buf[64];
	phandle_t input;

	if (OF_getprop(opts, inputdev, buf, sizeof(buf)) == -1)
		return (ENXIO);
	input = OF_finddevice(buf);
	if (input == -1)
		return (ENXIO);
	if (OF_getprop(opts, outputdev, buf, sizeof(buf)) == -1)
		return (ENXIO);
	if (OF_finddevice(buf) != input)
		return (ENXIO);

	*result = input;
	return (0);
}

int
uart_cpu_getdev(int devtype, struct uart_devinfo *di)
{
	char buf[64];
	struct uart_class *class;
	phandle_t input, opts;
	int error;

	class = &uart_z8530_class;
	if (class == NULL)
		return (ENXIO);

	if ((opts = OF_finddevice("/options")) == -1)
		return (ENXIO);
	switch (devtype) {
	case UART_DEV_CONSOLE:
		if (ofw_get_uart_console(opts, &input, "input-device",
		    "output-device")) {
			/*
			 * At least some G5 Xserves require that we
			 * probe input-device-1 as well
			 */
	
			if (ofw_get_uart_console(opts, &input, "input-device-1",
			    "output-device-1"))
				return (ENXIO);
		}
		break;
	case UART_DEV_DBGPORT:
		if (!getenv_string("hw.uart.dbgport", buf, sizeof(buf)))
			return (ENXIO);
		input = OF_finddevice(buf);
		if (input == -1)
			return (ENXIO);
		break;
	default:
		return (EINVAL);
	}

	if (OF_getprop(input, "device_type", buf, sizeof(buf)) == -1)
		return (ENXIO);
	if (strcmp(buf, "serial") != 0)
		return (ENXIO);
	if (OF_getprop(input, "name", buf, sizeof(buf)) == -1)
		return (ENXIO);

	if (strcmp(buf, "ch-a") == 0) {
		class = &uart_z8530_class;
		di->bas.regshft = 4;
		di->bas.chan = 1;
	} else if (strcmp(buf,"serial") == 0) {
		class = &uart_ns8250_class;
		di->bas.regshft = 0;
		di->bas.chan = 0;
	} else
		return (ENXIO);

	error = OF_decode_addr(input, 0, &di->bas.bst, &di->bas.bsh);
	if (error)
		return (error);

	di->ops = uart_getops(class);

	if (OF_getprop(input, "clock-frequency", &di->bas.rclk, 
	    sizeof(di->bas.rclk)) == -1)
		di->bas.rclk = 230400;
	if (OF_getprop(input, "current-speed", &di->baudrate, 
	    sizeof(di->baudrate)) == -1)
		di->baudrate = 0;

	di->databits = 8;
	di->stopbits = 1;
	di->parity = UART_PARITY_NONE;
	return (0);
}