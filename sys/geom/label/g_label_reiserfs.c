
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

#define REISERFS_NEW_DISK_OFFSET 64 * 1024
#define REISERFS_OLD_DISK_OFFSET 8 * 1024
#define REISERFS_SUPER_MAGIC	"ReIsEr"

typedef struct reiserfs_sb {
	uint8_t		fake1[52];
	char		s_magic[10];
	uint8_t		fake2[10];
	uint16_t	s_version;
	uint8_t		fake3[26];
	char		s_volume_name[16];
} reiserfs_sb_t;

static reiserfs_sb_t *
g_label_reiserfs_read_super(struct g_consumer *cp, off_t offset)
{
	reiserfs_sb_t *fs;
	u_int secsize;

	secsize = cp->provider->sectorsize;

	if ((offset % secsize) != 0)
		return (NULL);

	fs = (reiserfs_sb_t *)g_read_data(cp, offset, secsize, NULL);
	if (fs == NULL)
		return (NULL);

	if (strncmp(fs->s_magic, REISERFS_SUPER_MAGIC,
	    strlen(REISERFS_SUPER_MAGIC)) != 0) {
		g_free(fs);
		return (NULL);
	}

	return (fs);
}

static void
g_label_reiserfs_taste(struct g_consumer *cp, char *label, size_t size)
{
	struct g_provider *pp;
	reiserfs_sb_t *fs;

	g_topology_assert_not();
	pp = cp->provider;
	label[0] = '\0';

	/* Try old format */
	fs = g_label_reiserfs_read_super(cp, REISERFS_OLD_DISK_OFFSET);
	if (fs == NULL) {
		/* Try new format */
		fs = g_label_reiserfs_read_super(cp, REISERFS_NEW_DISK_OFFSET);
	}
	if (fs == NULL)
		return;

	/* Check version */
	if (fs->s_version == 2) {
		G_LABEL_DEBUG(1, "reiserfs file system detected on %s.",
		    pp->name);
	} else {
		goto exit_free;
	}

	/* Check for volume label */
	if (fs->s_volume_name[0] == '\0')
		goto exit_free;

	/* Terminate label */
	fs->s_volume_name[sizeof(fs->s_volume_name) - 1] = '\0';
	strlcpy(label, fs->s_volume_name, size);

exit_free:
	g_free(fs);
}

struct g_label_desc g_label_reiserfs = {
	.ld_taste = g_label_reiserfs_taste,
	.ld_dir = "reiserfs",
	.ld_enabled = 1
};

G_LABEL_INIT(reiserfs, g_label_reiserfs, "Create device nodes for REISERFS volumes");