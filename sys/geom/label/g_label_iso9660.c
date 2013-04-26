
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
#include <sys/malloc.h>

#include <geom/geom.h>
#include <geom/label/g_label.h>

#define G_LABEL_ISO9660_DIR	"iso9660"

#define	ISO9660_MAGIC	"\x01" "CD001" "\x01\x00"
#define	ISO9660_OFFSET	0x8000
#define	VOLUME_LEN	32


static void
g_label_iso9660_taste(struct g_consumer *cp, char *label, size_t size)
{
	struct g_provider *pp;
	char *sector, *volume;
	int i;

	g_topology_assert_not();
	pp = cp->provider;
	label[0] = '\0';

	if ((ISO9660_OFFSET % pp->sectorsize) != 0)
		return;
	sector = (char *)g_read_data(cp, ISO9660_OFFSET, pp->sectorsize,
	    NULL);
	if (sector == NULL)
		return;
	if (bcmp(sector, ISO9660_MAGIC, sizeof(ISO9660_MAGIC) - 1) != 0) {
		g_free(sector);
		return;
	}
	G_LABEL_DEBUG(1, "ISO9660 file system detected on %s.", pp->name);
	volume = sector + 0x28;
	bzero(label, size);
	strlcpy(label, volume, MIN(size, VOLUME_LEN));
	g_free(sector);
	for (i = size - 1; i > 0; i--) {
		if (label[i] == '\0')
			continue;
		else if (label[i] == ' ')
			label[i] = '\0';
		else
			break;
	}
}

struct g_label_desc g_label_iso9660 = {
	.ld_taste = g_label_iso9660_taste,
	.ld_dir = G_LABEL_ISO9660_DIR,
	.ld_enabled = 1
};

G_LABEL_INIT(iso9660, g_label_iso9660, "Create device nodes for ISO9660 volume names");