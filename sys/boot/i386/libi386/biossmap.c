
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

/*
 * Obtain memory configuration information from the BIOS
 */
#include <stand.h>
#include <sys/param.h>
#include <sys/linker.h>
#include <sys/queue.h>
#include <sys/stddef.h>
#include <machine/metadata.h>
#include <machine/pc/bios.h>
#include "bootstrap.h"
#include "libi386.h"
#include "btxv86.h"

struct smap_buf {
	struct bios_smap	smap;
	uint32_t		xattr;	/* Extended attribute from ACPI 3.0 */
	STAILQ_ENTRY(smap_buf)	bufs;
};

#define	SMAP_BUFSIZE		offsetof(struct smap_buf, bufs)

static struct bios_smap		*smapbase;
static uint32_t			*smapattr;
static u_int			smaplen;

void
bios_getsmap(void)
{
	struct smap_buf		buf;
	STAILQ_HEAD(smap_head, smap_buf) head =
	    STAILQ_HEAD_INITIALIZER(head);
	struct smap_buf		*cur, *next;
	u_int			n, x;

	STAILQ_INIT(&head);
	n = 0;
	x = 0;
	v86.ebx = 0;
	do {
		v86.ctl = V86_FLAGS;
		v86.addr = 0x15;
		v86.eax = 0xe820;	/* int 0x15 function 0xe820 */
		v86.ecx = SMAP_BUFSIZE;
		v86.edx = SMAP_SIG;
		v86.es = VTOPSEG(&buf);
		v86.edi = VTOPOFF(&buf);
		v86int();
		if (V86_CY(v86.efl) || v86.eax != SMAP_SIG ||
		    v86.ecx < sizeof(buf.smap) || v86.ecx > SMAP_BUFSIZE)
			break;

		next = malloc(sizeof(*next));
		if (next == NULL)
			break;
		next->smap = buf.smap;
		if (v86.ecx == SMAP_BUFSIZE) {
			next->xattr = buf.xattr;
			x++;
		}
		STAILQ_INSERT_TAIL(&head, next, bufs);
		n++;
	} while (v86.ebx != 0);
	smaplen = n;

	if (smaplen > 0) {
		smapbase = malloc(smaplen * sizeof(*smapbase));
		if (smapbase != NULL) {
			n = 0;
			STAILQ_FOREACH(cur, &head, bufs)
				smapbase[n++] = cur->smap;
		}
		if (smaplen == x) {
			smapattr = malloc(smaplen * sizeof(*smapattr));
			if (smapattr != NULL) {
				n = 0;
				STAILQ_FOREACH(cur, &head, bufs)
					smapattr[n++] = cur->xattr &
					    SMAP_XATTR_MASK;
			}
		} else
			smapattr = NULL;
		cur = STAILQ_FIRST(&head);
		while (cur != NULL) {
			next = STAILQ_NEXT(cur, bufs);
			free(cur);
			cur = next;
		}
	}
}

void
bios_addsmapdata(struct preloaded_file *kfp)
{
	size_t			size;

	if (smapbase == NULL || smaplen == 0)
		return;
	size = smaplen * sizeof(*smapbase);
	file_addmetadata(kfp, MODINFOMD_SMAP, size, smapbase);
	if (smapattr != NULL) {
		size = smaplen * sizeof(*smapattr);
		file_addmetadata(kfp, MODINFOMD_SMAP_XATTR, size, smapattr);
	}
}

COMMAND_SET(smap, "smap", "show BIOS SMAP", command_smap);

static int
command_smap(int argc, char *argv[])
{
	u_int			i;

	if (smapbase == NULL || smaplen == 0)
		return (CMD_ERROR);
	if (smapattr != NULL)
		for (i = 0; i < smaplen; i++)
			printf("SMAP type=%02x base=%016llx len=%016llx attr=%02x\n",
			    (unsigned int)smapbase[i].type,
			    (unsigned long long)smapbase[i].base,
			    (unsigned long long)smapbase[i].length,
			    (unsigned int)smapattr[i]);
	else
		for (i = 0; i < smaplen; i++)
			printf("SMAP type=%02x base=%016llx len=%016llx\n",
			    (unsigned int)smapbase[i].type,
			    (unsigned long long)smapbase[i].base,
			    (unsigned long long)smapbase[i].length);
	return (CMD_OK);
}