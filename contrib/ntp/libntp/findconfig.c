
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
	/* All keyed by initial target being a directory */	(void) strcpy(result, base);
	if (stat(result, &sbuf) == 0) {
		if (S_ISDIR(sbuf.st_mode)) {

			/* First choice is my hostname */
			if (gethostname(hostname, BUFSIZ) >= 0) {
				(void) sprintf(result, "%s/%s", base, hostname);
				if (stat(result, &sbuf) == 0) {
					goto outahere;
				} else {

					/* Second choice is of form default.835 */
					(void) uname(&unamebuf);
					if (strncmp(unamebuf.machine, "9000/", 5) == 0)
					    cp = unamebuf.machine + 5;
					else
					    cp = unamebuf.machine;
					(void) sprintf(result, "%s/default.%s", base, cp);
					if (stat(result, &sbuf) == 0) {
						goto outahere;
					} else {

						/* Last choice is just default */
						(void) sprintf(result, "%s/default", base);
						if (stat(result, &sbuf) == 0) {
							goto outahere;
						} else {
							(void) strcpy(result, "/not/found");
						}
					}
				}
			} 
		} 
	}
    outahere:
	return(result);
}
#else
#include "ntp_stdlib.h"

const char *
FindConfig(
	const char *base
	)
{
	return base;
}
#endif