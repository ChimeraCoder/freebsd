
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
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <cam/scsi/scsi_all.h>
#include <cam/scsi/scsi_enc.h>

#include "eltsub.h"

int
main(int a, char **v)
{
	encioc_element_t *objp;
	encioc_elm_status_t ob;
	encioc_elm_desc_t objd;
	encioc_elm_devnames_t objdn;
	int fd, nobj, f, i, verbose, quiet, errors;
	u_char estat;

	if (a < 2) {
		fprintf(stderr, "usage: %s [ -v ] device [ device ... ]\n", *v);
		return (1);
	}
	errors = quiet = verbose = 0;
	if (strcmp(v[1], "-V") == 0) {
		verbose = 2;
		v++;
	} else if (strcmp(v[1], "-v") == 0) {
		verbose = 1;
		v++;
	} else if (strcmp(v[1], "-q") == 0) {
		quiet = 1;
		verbose = 0;
		v++;
	}
	while (*++v) {
			
		fd = open(*v, O_RDONLY);
		if (fd < 0) {
			perror(*v);
			continue;
		}
		if (ioctl(fd, ENCIOC_GETNELM, (caddr_t) &nobj) < 0) {
			perror("ENCIOC_GETNELM");
			(void) close(fd);
			continue;
		}
		if (ioctl(fd, ENCIOC_GETENCSTAT, (caddr_t) &estat) < 0) {
			perror("ENCIOC_GETENCSTAT");
			(void) close(fd);
			continue;
		}
		if ((verbose == 0 || quiet == 1) && estat == 0) {
			if (quiet == 0)
				fprintf(stdout, "%s: Enclosure OK\n", *v);
			(void) close(fd);
			continue;
		}
		fprintf(stdout, "%s: Enclosure Status ", *v);
		if (estat == 0) {
			fprintf(stdout, "<OK");
		} else {
			errors++;
			f = '<';
			if (estat & SES_ENCSTAT_INFO) {
				fprintf(stdout, "%cINFO", f);
				f = ',';
			}
			if (estat & SES_ENCSTAT_NONCRITICAL) {
				fprintf(stdout, "%cNONCRITICAL", f);
				f = ',';
			}
			if (estat & SES_ENCSTAT_CRITICAL) {
				fprintf(stdout, "%cCRITICAL", f);
				f = ',';
			}
			if (estat & SES_ENCSTAT_UNRECOV) {
				fprintf(stdout, "%cUNRECOV", f);
				f = ',';
			}
		}
		fprintf(stdout, ">\n");
		objp = calloc(nobj, sizeof (encioc_element_t));
		if (objp == NULL) {
			perror("calloc");
			(void) close(fd);
			continue;
		}
                if (ioctl(fd, ENCIOC_GETELMMAP, (caddr_t) objp) < 0) {
                        perror("ENCIOC_GETELMMAP");
                        (void) close(fd);
                        continue;
                }
		for (i = 0; i < nobj; i++) {
			ob.elm_idx = objp[i].elm_idx;
			if (ioctl(fd, ENCIOC_GETELMSTAT, (caddr_t) &ob) < 0) {
				perror("ENCIOC_GETELMSTAT");
				(void) close(fd);
				break;
			}
			bzero(&objd, sizeof(objd));
			objd.elm_idx = objp[i].elm_idx;
			objd.elm_desc_len = UINT16_MAX;
			objd.elm_desc_str = calloc(UINT16_MAX, sizeof(char));
			if (objd.elm_desc_str == NULL) {
				perror("calloc");
				(void) close(fd);
				continue;
			}
			if (ioctl(fd, ENCIOC_GETELMDESC, (caddr_t)&objd) < 0) {
				perror("ENCIOC_GETELMDESC");
				(void) close(fd);
				break;
			}
			bzero(&objdn, sizeof(objdn));
			objdn.elm_idx = objp[i].elm_idx;
			objdn.elm_names_size = 128;
			objdn.elm_devnames = calloc(128, sizeof(char));
			if (objdn.elm_devnames == NULL) {
				perror("calloc");
				(void) close(fd);
				break;
			}
			/*
			 * This ioctl isn't critical and has a good chance
			 * of returning -1.
			 */
			(void)ioctl(fd, ENCIOC_GETELMDEVNAMES, (caddr_t)&objdn);
			fprintf(stdout, "Element 0x%x: %s", ob.elm_idx,
			    geteltnm(objp[i].elm_type));
			fprintf(stdout, ", %s",
			    stat2ascii(objp[i].elm_type, ob.cstat));
			if (objd.elm_desc_len > 0)
				fprintf(stdout, ", descriptor: '%s'",
				    objd.elm_desc_str);
			if (objdn.elm_names_len > 0)
				fprintf(stdout, ", dev: '%s'",
				    objdn.elm_devnames);
			fprintf(stdout, "\n");
			free(objdn.elm_devnames);
		}
		free(objp);
		(void) close(fd);
	}
	return (errors);
}