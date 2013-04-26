
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/alq.h>

#include "ah_decode.h"

#define	MAX_MARKERS	9

const char *markers[] = {  
        "AH_MARK_RESET",                  /* ar*Reset entry, bChannelChange */
        "AH_MARK_RESET_LINE",             /* ar*_reset.c, line %d */
        "AH_MARK_RESET_DONE",             /* ar*Reset exit, error code */
        "AH_MARK_CHIPRESET",              /* ar*ChipReset, channel num */
        "AH_MARK_PERCAL",                 /* ar*PerCalibration, channel num */
        "AH_MARK_SETCHANNEL",             /* ar*SetChannel, channel num */
        "AH_MARK_ANI_RESET",              /* ar*AniReset, opmode */
        "AH_MARK_ANI_POLL",               /* ar*AniReset, listen time */
        "AH_MARK_ANI_CONTROL",            /* ar*AniReset, cmd */
};

static void
op_read(struct athregrec *a)
{
        printf("read\t%.8x = %.8x\n", a->reg, a->val);
}

static void
op_write(struct athregrec *a)
{
        printf("write\t%.8x = %.8x\n", a->reg, a->val);
}

static void
op_device(struct athregrec *a)
{
        printf("device\t0x%x/0x%x\n", a->reg, a->val);
}

static void
op_mark(struct athregrec *a)
{
        const char *s = "UNKNOWN";
        if (a->reg <= MAX_MARKERS)
                s = markers[a->reg];

	printf("mark\t%s (%d): %d\n", s, a->reg, a->val);
}

int
main(int argc, const char *argv[])
{
	const char *file = argv[1];
	int fd;
	struct athregrec a;
	int r;

	if (argc < 2) {
		printf("usage: %s <ahq log>\n", argv[0]);
		exit(127);
	}

	fd = open(file, O_RDONLY);
	if (fd < 0) {
		perror("open"); 
		exit(127);
	}

	while (1) {
		r = read(fd, &a, sizeof(a));
		if (r != sizeof(a))
			break;
		switch (a.op) {
			case OP_READ:
				op_read(&a);
				break;
			case OP_WRITE:
				op_write(&a);
				break;
			case OP_DEVICE:
				op_device(&a);
				break;
			case OP_MARK:
				op_mark(&a);
				break;
			default:
				printf("op: %d; reg: 0x%x; val: 0x%x\n",
				    a.op, a.reg, a.val);
		}
	}
	close(fd);
}