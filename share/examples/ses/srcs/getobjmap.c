
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

#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <cam/scsi/scsi_all.h>
#include <cam/scsi/scsi_ses.h>

#include "eltsub.h"

int
main(int a, char **v)
{
	ses_object *objp;
	int nobj, fd, i;

	while (*++v) {
		fd = open(*v, O_RDONLY);
		if (fd < 0) {
			perror(*v);
			continue;
		}
		if (ioctl(fd, SESIOC_GETNOBJ, (caddr_t) &nobj) < 0) {
			perror("SESIOC_GETNOBJ");
			(void) close(fd);
			continue;
		}
		fprintf(stdout, "%s: %d objects\n", *v, nobj);
		if (nobj == 0) {
			(void) close(fd);
			continue;
		}
		objp = calloc(nobj, sizeof (ses_object));
		if (objp == NULL) {
			perror("calloc");
			(void) close(fd);
			continue;
		}
		if (ioctl(fd, SESIOC_GETOBJMAP, (caddr_t) objp) < 0) {
			perror("SESIOC_GETOBJMAP");
			(void) close(fd);
			continue;
		}
		for (i = 0; i < nobj; i++) {
			printf(" Object %d: ID 0x%x Type '%s'\n", i,
			    objp[i].obj_id, geteltnm((int)objp[i].object_type));
		}
		free(objp);
		(void) close(fd);
	}
	return (0);
}