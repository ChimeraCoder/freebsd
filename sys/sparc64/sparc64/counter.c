
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
#include <sys/bus.h>
#include <sys/malloc.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <sys/timetc.h>

#include <machine/bus.h>
#include <machine/bus_common.h>

#define	COUNTER_MASK	((1 << 29) - 1)
#define	COUNTER_FREQ	1000000
#define	COUNTER_QUALITY	100

/* Bits in the limit register. */
#define	CTLR_INTEN	(1U << 31)	/* Enable timer interrupts */
#define	CTLR_RELOAD	(1U << 30)	/* Zero counter on write to limit reg */
#define	CTLR_PERIODIC	(1U << 29)	/* Wrap to 0 if limit is reached */

/* Offsets of the registers for the two counters. */
#define	CTR_CT0		0x00
#define	CTR_CT1		0x10

/* Register offsets from the base address. */
#define	CTR_COUNT	0x00
#define	CTR_LIMIT	0x08


static timecounter_get_t counter_get_timecount;

struct ct_softc {
	bus_space_tag_t		sc_tag;
	bus_space_handle_t	sc_handle;
	bus_addr_t		sc_offset;
};


/*
 * This is called from the psycho and sbus drivers.  It does not directly
 * attach to the nexus because it shares register space with the bridge in
 * question.
 */
void
sparc64_counter_init(const char *name, bus_space_tag_t tag,
    bus_space_handle_t handle, bus_addr_t offset)
{
	struct timecounter *tc;
	struct ct_softc *sc;

	printf("initializing counter-timer\n");
	/*
	 * Turn off interrupts from both counters.  Set the limit to the
	 * maximum value (although that should not change anything with
	 * CTLR_INTEN and CTLR_PERIODIC off).
	 */
	bus_space_write_8(tag, handle, offset + CTR_CT0 + CTR_LIMIT,
	    COUNTER_MASK);
	bus_space_write_8(tag, handle, offset + CTR_CT1 + CTR_LIMIT,
	    COUNTER_MASK);
	/* Register as a time counter. */
	tc = malloc(sizeof(*tc), M_DEVBUF, M_WAITOK | M_ZERO);
	sc = malloc(sizeof(*sc), M_DEVBUF, M_WAITOK);
	sc->sc_tag = tag;
	sc->sc_handle = handle;
	sc->sc_offset = offset + CTR_CT0;
	tc->tc_get_timecount = counter_get_timecount;
	tc->tc_counter_mask = COUNTER_MASK;
	tc->tc_frequency = COUNTER_FREQ;
	tc->tc_name = strdup(name, M_DEVBUF);
	tc->tc_priv = sc;
	tc->tc_quality = COUNTER_QUALITY;
	tc_init(tc);
}

static unsigned int
counter_get_timecount(struct timecounter *tc)
{
	struct ct_softc *sc;

	sc = tc->tc_priv;
	return (bus_space_read_8(sc->sc_tag, sc->sc_handle, sc->sc_offset));
}