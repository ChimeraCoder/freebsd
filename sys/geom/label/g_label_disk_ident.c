
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
#include <geom/geom_disk.h>
#include <geom/label/g_label.h>
#include <geom/multipath/g_multipath.h>


#define G_LABEL_DISK_IDENT_DIR	"diskid"

static char* classes_pass[] = { G_DISK_CLASS_NAME, G_MULTIPATH_CLASS_NAME,
    NULL };

static void
g_label_disk_ident_taste(struct g_consumer *cp, char *label, size_t size)
{
	struct g_class *cls;
	char ident[100];
	int ident_len, found, i;

	g_topology_assert_not();
	label[0] = '\0';

	cls = cp->provider->geom->class;

	/* 
	 * Get the GEOM::ident string, and construct a label in the format
	 * "CLASS_NAME-ident"
	 */
	ident_len = sizeof(ident);
	if (g_io_getattr("GEOM::ident", cp, &ident_len, ident) == 0) {
		if (ident_len == 0 || ident[0] == '\0')
			return;
		for (i = 0, found = 0; classes_pass[i] != NULL; i++)
			if (strcmp(classes_pass[i], cls->name) == 0) {
				found = 1;
				break;
			}
		if (!found)
			return;
		/*
		 * We can safely ignore the result of snprintf(): the label
		 * will simply be truncated, which at most is only annoying.
		 */
		(void)snprintf(label, size, "%s-%s", cls->name, ident);
	}
}

struct g_label_desc g_label_disk_ident = {
	.ld_taste = g_label_disk_ident_taste,
	.ld_dir = G_LABEL_DISK_IDENT_DIR,
	.ld_enabled = 1
};

G_LABEL_INIT(disk_ident, g_label_disk_ident, "Create device nodes for drives "
    "which export a disk identification string");