
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
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/module.h>
#include <sys/tty.h>
#include <machine/bus.h>
#include <sys/timepps.h>

#include <dev/sio/siovar.h>

#include <isa/isavar.h>

static	int	sio_isa_attach(device_t dev);
static	int	sio_isa_probe(device_t dev);

static device_method_t sio_isa_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		sio_isa_probe),
	DEVMETHOD(device_attach,	sio_isa_attach),
	DEVMETHOD(device_detach,	siodetach),

	{ 0, 0 }
};

static driver_t sio_isa_driver = {
	sio_driver_name,
	sio_isa_methods,
	0,
};

static struct isa_pnp_id sio_ids[] = {
	{0x0100e4a5, "RSA-98III"},
	{0x4180a3b8, NULL},	/* NEC8041 - PC-9821CB-B04 */
	{0x0181a3b8, NULL},	/* NEC8101 - PC-9821CB2-B04 */
	{0x5181a3b8, NULL},	/* NEC8151 - Internal FAX/Modem for Cx3, Cb3 */
	{0x9181a3b8, NULL},	/* NEC8191 - NEC PC-9801-120 */
	{0xe181a3b8, NULL},	/* NEC81E1 - Internal FAX/Modem */
	{0x1182a3b8, NULL},	/* NEC8211 - PC-9801-123 */
	{0x3182a3b8, NULL},	/* NEC8231 - Internal FAX/Modem(Voice) */
	{0x4182a3b8, NULL},	/* NEC8241 - NEC PC-9821NR-B05 */
	{0x5182a3b8, NULL},	/* NEC8251 - Internel FAX/Modem */
	{0x7182a3b8, NULL},	/* NEC8271 - NEC PC-9801-125 */
	{0x11802fbf, NULL},	/* OYO8011 - Internal FAX/Modem for ValueStar(Ring) */
	{0}
};

static int
sio_isa_probe(dev)
	device_t	dev;
{
	int	logical_id;

	/* Check isapnp ids */
	if (ISA_PNP_PROBE(device_get_parent(dev), dev, sio_ids) == ENXIO)
		return (ENXIO);

	logical_id = isa_get_logicalid(dev);
	switch (logical_id) {
	case 0x0100e4a5:	/* RSA-98III */
		SET_FLAG(dev, SET_IFTYPE(COM_IF_RSA98III));
		break;
	case 0x4180a3b8:	/* PC-9821CB-B04 */
	case 0x0181a3b8:	/* PC-9821CB2-B04 */
	case 0x5181a3b8:	/* for Cx3, Cb3 internal */
	case 0x9181a3b8:	/* PC-9801-120 */
	case 0xe181a3b8:	/* Internal FAX/Modem */
	case 0x1182a3b8:	/* PC-9801-123 */
	case 0x3182a3b8:	/* FAX/Voice/Modem internal */
	case 0x4182a3b8:	/* PC-9821NR-B05 */
	case 0x5182a3b8:	/* FAX/Modem internal */
	case 0x7182a3b8:	/* PC-9801-125 */
	case 0x11802fbf:	/* for ValueStar internal */
		SET_FLAG(dev, SET_IFTYPE(COM_IF_NS16550));
		break;
	}

	return (sioprobe(dev, 0, 0UL, 0));
}

static int
sio_isa_attach(dev)
	device_t	dev;
{
	return (sioattach(dev, 0, 0UL));
}

DRIVER_MODULE(sio, isa, sio_isa_driver, sio_devclass, 0, 0);