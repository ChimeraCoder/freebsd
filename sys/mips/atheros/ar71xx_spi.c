
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

#include <sys/bus.h>
#include <sys/interrupt.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/rman.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_extern.h>

#include <machine/bus.h>
#include <machine/cpu.h>
#include <machine/pmap.h>

#include <dev/spibus/spi.h>
#include <dev/spibus/spibusvar.h>
#include "spibus_if.h"

#include <mips/atheros/ar71xxreg.h>

#undef AR71XX_SPI_DEBUG
#ifdef AR71XX_SPI_DEBUG
#define dprintf printf
#else
#define dprintf(x, arg...)
#endif

/*
 * register space access macros
 */
#define SPI_WRITE(sc, reg, val)	do {	\
		bus_write_4(sc->sc_mem_res, (reg), (val)); \
	} while (0)

#define SPI_READ(sc, reg)	 bus_read_4(sc->sc_mem_res, (reg))

#define SPI_SET_BITS(sc, reg, bits)	\
	SPI_WRITE(sc, reg, SPI_READ(sc, (reg)) | (bits))

#define SPI_CLEAR_BITS(sc, reg, bits)	\
	SPI_WRITE(sc, reg, SPI_READ(sc, (reg)) & ~(bits))

struct ar71xx_spi_softc {
	device_t		sc_dev;
	struct resource		*sc_mem_res;
	uint32_t		sc_reg_ctrl;
};

static int
ar71xx_spi_probe(device_t dev)
{
	device_set_desc(dev, "AR71XX SPI");
	return (0);
}

static int
ar71xx_spi_attach(device_t dev)
{
	struct ar71xx_spi_softc *sc = device_get_softc(dev);
	int rid;

	sc->sc_dev = dev;
        rid = 0;
	sc->sc_mem_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY, &rid, 
	    RF_ACTIVE);
	if (!sc->sc_mem_res) {
		device_printf(dev, "Could not map memory\n");
		return (ENXIO);
	}


	SPI_WRITE(sc, AR71XX_SPI_FS, 1);
	sc->sc_reg_ctrl  = SPI_READ(sc, AR71XX_SPI_CTRL);
	SPI_WRITE(sc, AR71XX_SPI_CTRL, 0x43);
	SPI_WRITE(sc, AR71XX_SPI_IO_CTRL, SPI_IO_CTRL_CSMASK);

	device_add_child(dev, "spibus", 0);
	return (bus_generic_attach(dev));
}

static void
ar71xx_spi_chip_activate(struct ar71xx_spi_softc *sc, int cs)
{
	uint32_t ioctrl = SPI_IO_CTRL_CSMASK;
	/*
	 * Put respective CSx to low
	 */
	ioctrl &= ~(SPI_IO_CTRL_CS0 << cs);

	SPI_WRITE(sc, AR71XX_SPI_IO_CTRL, ioctrl);
}

static void
ar71xx_spi_chip_deactivate(struct ar71xx_spi_softc *sc, int cs)
{
	/*
	 * Put all CSx to high
	 */
	SPI_WRITE(sc, AR71XX_SPI_IO_CTRL, SPI_IO_CTRL_CSMASK);
}

static uint8_t
ar71xx_spi_txrx(struct ar71xx_spi_softc *sc, int cs, uint8_t data)
{
	int bit;
	/* CS0 */
	uint32_t ioctrl = SPI_IO_CTRL_CSMASK;
	/*
	 * low-level for selected CS
	 */
	ioctrl &= ~(SPI_IO_CTRL_CS0 << cs);

	uint32_t iod, rds;
	for (bit = 7; bit >=0; bit--) {
		if (data & (1 << bit))
			iod = ioctrl | SPI_IO_CTRL_DO;
		else
			iod = ioctrl & ~SPI_IO_CTRL_DO;
		SPI_WRITE(sc, AR71XX_SPI_IO_CTRL, iod);
		SPI_WRITE(sc, AR71XX_SPI_IO_CTRL, iod | SPI_IO_CTRL_CLK);
	}

	/*
	 * Provide falling edge for connected device by clear clock bit.
	 */
	SPI_WRITE(sc, AR71XX_SPI_IO_CTRL, iod);
	rds = SPI_READ(sc, AR71XX_SPI_RDS);

	return (rds & 0xff);
}

static int
ar71xx_spi_transfer(device_t dev, device_t child, struct spi_command *cmd)
{
	struct ar71xx_spi_softc *sc;
	uint8_t *buf_in, *buf_out;
	struct spibus_ivar *devi = SPIBUS_IVAR(child);
	int i;

	sc = device_get_softc(dev);

	ar71xx_spi_chip_activate(sc, devi->cs);

	KASSERT(cmd->tx_cmd_sz == cmd->rx_cmd_sz, 
	    ("TX/RX command sizes should be equal"));
	KASSERT(cmd->tx_data_sz == cmd->rx_data_sz, 
	    ("TX/RX data sizes should be equal"));

	/*
	 * Transfer command
	 */
	buf_out = (uint8_t *)cmd->tx_cmd;
	buf_in = (uint8_t *)cmd->rx_cmd;
	for (i = 0; i < cmd->tx_cmd_sz; i++)
		buf_in[i] = ar71xx_spi_txrx(sc, devi->cs, buf_out[i]);

	/*
	 * Receive/transmit data (depends on  command)
	 */
	buf_out = (uint8_t *)cmd->tx_data;
	buf_in = (uint8_t *)cmd->rx_data;
	for (i = 0; i < cmd->tx_data_sz; i++)
		buf_in[i] = ar71xx_spi_txrx(sc, devi->cs, buf_out[i]);

	ar71xx_spi_chip_deactivate(sc, devi->cs);

	return (0);
}

static int
ar71xx_spi_detach(device_t dev)
{
	struct ar71xx_spi_softc *sc = device_get_softc(dev);

	SPI_WRITE(sc, AR71XX_SPI_CTRL, sc->sc_reg_ctrl);
	SPI_WRITE(sc, AR71XX_SPI_FS, 0);

	if (sc->sc_mem_res)
		bus_release_resource(dev, SYS_RES_MEMORY, 0, sc->sc_mem_res);

	return (0);
}

static device_method_t ar71xx_spi_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		ar71xx_spi_probe),
	DEVMETHOD(device_attach,	ar71xx_spi_attach),
	DEVMETHOD(device_detach,	ar71xx_spi_detach),

	DEVMETHOD(spibus_transfer,	ar71xx_spi_transfer),

	{0, 0}
};

static driver_t ar71xx_spi_driver = {
	"spi",
	ar71xx_spi_methods,
	sizeof(struct ar71xx_spi_softc),
};

static devclass_t ar71xx_spi_devclass;

DRIVER_MODULE(ar71xx_spi, nexus, ar71xx_spi_driver, ar71xx_spi_devclass, 0, 0);