
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

#include "opt_mac.h"

#include <sys/param.h>
#include <sys/module.h>
#include <sys/sysctl.h>
#include <sys/systm.h>

#include <vm/uma.h>

#include <security/mac/mac_framework.h>
#include <security/mac/mac_internal.h>
#include <security/mac/mac_policy.h>

/*
 * zone_label is the UMA zone from which most labels are allocated.  Label
 * structures are initialized to zero bytes so that policies see a NULL/0
 * slot on first use, even if the policy is loaded after the label is
 * allocated for an object.
 */
static uma_zone_t	zone_label;

static int	mac_labelzone_ctor(void *mem, int size, void *arg, int flags);
static void	mac_labelzone_dtor(void *mem, int size, void *arg);

void
mac_labelzone_init(void)
{

	zone_label = uma_zcreate("MAC labels", sizeof(struct label),
	    mac_labelzone_ctor, mac_labelzone_dtor, NULL, NULL,
	    UMA_ALIGN_PTR, 0);
}

/*
 * mac_init_label() and mac_destroy_label() are exported so that they can be
 * used in mbuf tag initialization, where labels are not slab allocated from
 * the zone_label zone.
 */
void
mac_init_label(struct label *label)
{

	bzero(label, sizeof(*label));
	label->l_flags = MAC_FLAG_INITIALIZED;
}

void
mac_destroy_label(struct label *label)
{

	KASSERT(label->l_flags & MAC_FLAG_INITIALIZED,
	    ("destroying uninitialized label"));

#ifdef DIAGNOSTIC
	bzero(label, sizeof(*label));
#else
	label->l_flags &= ~MAC_FLAG_INITIALIZED;
#endif
}


static int
mac_labelzone_ctor(void *mem, int size, void *arg, int flags)
{
	struct label *label;

	KASSERT(size == sizeof(*label), ("mac_labelzone_ctor: wrong size\n"));
	label = mem;
	mac_init_label(label);
	return (0);
}

static void
mac_labelzone_dtor(void *mem, int size, void *arg)
{
	struct label *label;

	KASSERT(size == sizeof(*label), ("mac_labelzone_dtor: wrong size\n"));
	label = mem;
	mac_destroy_label(label);
}

struct label *
mac_labelzone_alloc(int flags)
{

	return (uma_zalloc(zone_label, flags));
}

void
mac_labelzone_free(struct label *label)
{

	uma_zfree(zone_label, label);
}

/*
 * Functions used by policy modules to get and set label values.
 */
intptr_t
mac_label_get(struct label *l, int slot)
{

	KASSERT(l != NULL, ("mac_label_get: NULL label"));

	return (l->l_perpolicy[slot]);
}

void
mac_label_set(struct label *l, int slot, intptr_t v)
{

	KASSERT(l != NULL, ("mac_label_set: NULL label"));

	l->l_perpolicy[slot] = v;
}