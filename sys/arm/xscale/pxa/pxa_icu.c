
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
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/malloc.h>
#include <sys/rman.h>
#include <sys/timetc.h>
#include <machine/bus.h>
#include <machine/intr.h>

#include <arm/xscale/pxa/pxavar.h>
#include <arm/xscale/pxa/pxareg.h>

struct pxa_icu_softc {
	struct resource	*	pi_res[1];
	bus_space_tag_t		pi_bst;
	bus_space_handle_t	pi_bsh;
};

static struct resource_spec pxa_icu_spec[] = {
	{ SYS_RES_MEMORY,	0,	RF_ACTIVE },
	{ -1, 0 }
};

static struct pxa_icu_softc *pxa_icu_softc = NULL;

static int	pxa_icu_probe(device_t);
static int	pxa_icu_attach(device_t);

uint32_t	pxa_icu_get_icip(void);
void		pxa_icu_clear_icip(int);
uint32_t	pxa_icu_get_icfp(void);
void		pxa_icu_clear_icfp(int);
uint32_t	pxa_icu_get_icmr(void);
void		pxa_icu_set_icmr(uint32_t);
uint32_t	pxa_icu_get_iclr(void);
void		pxa_icu_set_iclr(uint32_t);
uint32_t	pxa_icu_get_icpr(void);
void		pxa_icu_idle_enable(void);
void		pxa_icu_idle_disable(void);

extern uint32_t	pxa_gpio_intr_flags[];

static int
pxa_icu_probe(device_t dev)
{

	device_set_desc(dev, "Interrupt Controller");
	return (0);
}

static int
pxa_icu_attach(device_t dev)
{
	int	error;
	struct	pxa_icu_softc *sc;

	sc = (struct pxa_icu_softc *)device_get_softc(dev);

	if (pxa_icu_softc != NULL)
		return (ENXIO);
	pxa_icu_softc = sc;

	error = bus_alloc_resources(dev, pxa_icu_spec, sc->pi_res);
	if (error) {
		device_printf(dev, "could not allocate resources\n");
		return (ENXIO);
	}

	sc->pi_bst = rman_get_bustag(sc->pi_res[0]);
	sc->pi_bsh = rman_get_bushandle(sc->pi_res[0]);

	/* Disable all interrupts. */
	pxa_icu_set_icmr(0);

	/* Route all interrupts to IRQ rather than FIQ. */
	pxa_icu_set_iclr(0);

	/* XXX: This should move to configure_final or something. */
	enable_interrupts(I32_bit|F32_bit);

	return (0);
}

static device_method_t pxa_icu_methods[] = {
	DEVMETHOD(device_probe, pxa_icu_probe),
	DEVMETHOD(device_attach, pxa_icu_attach),

	{0, 0}
};

static driver_t pxa_icu_driver = {
	"icu",
	pxa_icu_methods,
	sizeof(struct pxa_icu_softc),
};

static devclass_t pxa_icu_devclass;

DRIVER_MODULE(pxaicu, pxa, pxa_icu_driver, pxa_icu_devclass, 0, 0);

int
arm_get_next_irq(int last __unused)
{
	int	irq;

	if ((irq = pxa_icu_get_icip()) != 0) {
		return (ffs(irq) - 1);
	}

	return (pxa_gpio_get_next_irq());
}

void
arm_mask_irq(uintptr_t nb)
{
	uint32_t	mr;

	if (nb >= IRQ_GPIO0) {
		pxa_gpio_mask_irq(nb);
		return;
	}

	mr = pxa_icu_get_icmr();
	mr &= ~(1 << nb);
	pxa_icu_set_icmr(mr);
}

void
arm_unmask_irq(uintptr_t nb)
{
	uint32_t	mr;

	if (nb >= IRQ_GPIO0) {
		pxa_gpio_unmask_irq(nb);
		return;
	}

	mr = pxa_icu_get_icmr();
	mr |= (1 << nb);
	pxa_icu_set_icmr(mr);
}

uint32_t
pxa_icu_get_icip()
{

	return (bus_space_read_4(pxa_icu_softc->pi_bst,
	    pxa_icu_softc->pi_bsh, ICU_IP));
}

void
pxa_icu_clear_icip(int irq)
{

	bus_space_write_4(pxa_icu_softc->pi_bst,
	    pxa_icu_softc->pi_bsh, ICU_IP, (1 << irq));
}

uint32_t
pxa_icu_get_icfp()
{

	return (bus_space_read_4(pxa_icu_softc->pi_bst,
	    pxa_icu_softc->pi_bsh, ICU_FP));
}

void
pxa_icu_clear_icfp(int irq)
{

	bus_space_write_4(pxa_icu_softc->pi_bst,
	    pxa_icu_softc->pi_bsh, ICU_FP, (1 << irq));
}

uint32_t
pxa_icu_get_icmr()
{

	return (bus_space_read_4(pxa_icu_softc->pi_bst,
	    pxa_icu_softc->pi_bsh, ICU_MR));
}

void
pxa_icu_set_icmr(uint32_t val)
{

	bus_space_write_4(pxa_icu_softc->pi_bst,
	    pxa_icu_softc->pi_bsh, ICU_MR, val);
}

uint32_t
pxa_icu_get_iclr()
{

	return (bus_space_read_4(pxa_icu_softc->pi_bst,
	    pxa_icu_softc->pi_bsh, ICU_LR));
}

void
pxa_icu_set_iclr(uint32_t val)
{

	bus_space_write_4(pxa_icu_softc->pi_bst,
	    pxa_icu_softc->pi_bsh, ICU_LR, val);
}

uint32_t
pxa_icu_get_icpr()
{

	return (bus_space_read_4(pxa_icu_softc->pi_bst,
	    pxa_icu_softc->pi_bsh, ICU_PR));
}

void
pxa_icu_idle_enable()
{

	bus_space_write_4(pxa_icu_softc->pi_bst,
	    pxa_icu_softc->pi_bsh, ICU_CR, 0x0);
}

void
pxa_icu_idle_disable()
{

	bus_space_write_4(pxa_icu_softc->pi_bst,
	    pxa_icu_softc->pi_bsh, ICU_CR, 0x1);
}