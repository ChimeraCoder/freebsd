
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

#ifdef lint
static const char sccsid[] = "@(#)disks.c	8.1 (Berkeley) 6/6/93";
#endif

#include <sys/types.h>
#include <sys/devicestat.h>
#include <sys/resource.h>

#include <ctype.h>
#include <devstat.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>

#include "systat.h"
#include "extern.h"
#include "devs.h"

typedef enum {
	DS_MATCHTYPE_NONE,
	DS_MATCHTYPE_SPEC,
	DS_MATCHTYPE_PATTERN
} last_match_type;

last_match_type last_type;
struct device_selection *dev_select;
long generation;
int num_devices, num_selected;
int num_selections;
long select_generation;
struct devstat_match *matches = NULL;
int num_matches = 0;
char **specified_devices;
int num_devices_specified = 0;

static int dsmatchselect(const char *args, devstat_select_mode select_mode,
			 int maxshowdevs, struct statinfo *s1);
static int dsselect(const char *args, devstat_select_mode select_mode,
		    int maxshowdevs, struct statinfo *s1);

int
dsinit(int maxshowdevs, struct statinfo *s1, struct statinfo *s2 __unused,
       struct statinfo *s3 __unused)
{

	/*
	 * Make sure that the userland devstat version matches the kernel
	 * devstat version.  If not, exit and print a message informing
	 * the user of his mistake.
	 */
	if (devstat_checkversion(NULL) < 0)
		errx(1, "%s", devstat_errbuf);

	generation = 0;
	num_devices = 0;
	num_selected = 0;
	num_selections = 0;
	select_generation = 0;
	last_type = DS_MATCHTYPE_NONE;

	if (devstat_getdevs(NULL, s1) == -1)
		errx(1, "%s", devstat_errbuf);

	num_devices = s1->dinfo->numdevs;
	generation = s1->dinfo->generation;

	dev_select = NULL;

	/*
	 * At this point, selectdevs will almost surely indicate that the
	 * device list has changed, so we don't look for return values of 0
	 * or 1.  If we get back -1, though, there is an error.
	 */
	if (devstat_selectdevs(&dev_select, &num_selected, &num_selections,
	    &select_generation, generation, s1->dinfo->devices, num_devices,
	    NULL, 0, NULL, 0, DS_SELECT_ADD, maxshowdevs, 0) == -1)
		errx(1, "%d %s", __LINE__, devstat_errbuf);

	return(1);
}

int
dscmd(const char *cmd, const char *args, int maxshowdevs, struct statinfo *s1)
{
	int retval;

	if (prefix(cmd, "display") || prefix(cmd, "add"))
		return(dsselect(args, DS_SELECT_ADDONLY, maxshowdevs, s1));
	if (prefix(cmd, "ignore") || prefix(cmd, "delete"))
		return(dsselect(args, DS_SELECT_REMOVE, maxshowdevs, s1));
	if (prefix(cmd, "show") || prefix(cmd, "only"))
		return(dsselect(args, DS_SELECT_ONLY, maxshowdevs, s1));
	if (prefix(cmd, "type") || prefix(cmd, "match"))
		return(dsmatchselect(args, DS_SELECT_ONLY, maxshowdevs, s1));
	if (prefix(cmd, "refresh")) {
		retval = devstat_selectdevs(&dev_select, &num_selected,
		    &num_selections, &select_generation, generation,
		    s1->dinfo->devices, num_devices,
		    (last_type ==DS_MATCHTYPE_PATTERN) ?  matches : NULL,
		    (last_type ==DS_MATCHTYPE_PATTERN) ?  num_matches : 0,
		    (last_type == DS_MATCHTYPE_SPEC) ?specified_devices : NULL,
		    (last_type == DS_MATCHTYPE_SPEC) ?num_devices_specified : 0,
		    (last_type == DS_MATCHTYPE_NONE) ?  DS_SELECT_ADD :
		    DS_SELECT_ADDONLY, maxshowdevs, 0);
		if (retval == -1) {
			warnx("%s", devstat_errbuf);
			return(0);
		} else if (retval == 1)
			return(2);
	}
	if (prefix(cmd, "drives")) {
		int i;
		move(CMDLINE, 0);
		clrtoeol();
		for (i = 0; i < num_devices; i++) {
			printw("%s%d ", s1->dinfo->devices[i].device_name,
			       s1->dinfo->devices[i].unit_number);
		}
		return(1);
	}
	return(0);
}

static int
dsmatchselect(const char *args, devstat_select_mode select_mode, int maxshowdevs,
	      struct statinfo *s1)
{
	char **tempstr, *tmpstr, *tmpstr1;
	char *tstr[100];
	int num_args = 0;
	int i;
	int retval = 0;

	/*
	 * Break the (pipe delimited) input string out into separate
	 * strings.
	 */
	tmpstr = tmpstr1 = strdup(args);
	for (tempstr = tstr, num_args  = 0;
	     (*tempstr = strsep(&tmpstr1, "|")) != NULL && (num_args < 100);
	     num_args++)
		if (**tempstr != '\0')
			if (++tempstr >= &tstr[100])
				break;
	free(tmpstr);

	if (num_args > 99) {
		warnx("dsmatchselect: too many match arguments");
		return(0);
	}

	/*
	 * If we've gone through the matching code before, clean out
	 * previously used memory.
	 */
	if (num_matches > 0) {
		free(matches);
		matches = NULL;
		num_matches = 0;
	}

	for (i = 0; i < num_args; i++) {
		if (devstat_buildmatch(tstr[i], &matches, &num_matches) != 0) {
			warnx("%s", devstat_errbuf);
			return(0);
		}
	}
	if (num_args > 0) {

		last_type = DS_MATCHTYPE_PATTERN;

		retval = devstat_selectdevs(&dev_select, &num_selected,
		    &num_selections, &select_generation, generation,
		    s1->dinfo->devices, num_devices, matches, num_matches,
		    NULL, 0, select_mode, maxshowdevs, 0);
		if (retval == -1)
			err(1, "device selection error");
		else if (retval == 1)
			return(2);
	}
	return(1);
}

static int
dsselect(const char *args, devstat_select_mode select_mode, int maxshowdevs,
	 struct statinfo *s1)
{
	char *cp, *tmpstr, *tmpstr1, *buffer;
	int i;
	int retval = 0;

	/*
	 * If we've gone through this code before, free previously
	 * allocated resources.
	 */
	if (num_devices_specified > 0) {
		for (i = 0; i < num_devices_specified; i++)
			free(specified_devices[i]);
		free(specified_devices);
		specified_devices = NULL;
		num_devices_specified = 0;
	}

	/* do an initial malloc */
	specified_devices = (char **)malloc(sizeof(char *));

	tmpstr = tmpstr1 = strdup(args);
	cp = strchr(tmpstr1, '\n');
	if (cp)
		*cp = '\0';
	for (;;) {
		for (cp = tmpstr1; *cp && isspace(*cp); cp++)
			;
		tmpstr1 = cp;
		for (; *cp && !isspace(*cp); cp++)
			;
		if (*cp)
			*cp++ = '\0';
		if (cp - args == 0)
			break;
		for (i = 0; i < num_devices; i++) {
			asprintf(&buffer, "%s%d", dev_select[i].device_name,
				dev_select[i].unit_number);
			if (strcmp(buffer, tmpstr1) == 0) {

				num_devices_specified++;

				specified_devices =(char **)realloc(
						specified_devices,
						sizeof(char *) *
						num_devices_specified);
				specified_devices[num_devices_specified -1]=
					strdup(tmpstr1);
				free(buffer);

				break;
			}
			else
				free(buffer);
		}
		if (i >= num_devices)
			error("%s: unknown drive", args);
		args = cp;
	}
	free(tmpstr);

	if (num_devices_specified > 0) {
		last_type = DS_MATCHTYPE_SPEC;

		retval = devstat_selectdevs(&dev_select, &num_selected,
		    &num_selections, &select_generation, generation,
		    s1->dinfo->devices, num_devices, NULL, 0,
		    specified_devices, num_devices_specified,
		    select_mode, maxshowdevs, 0);
		if (retval == -1)
			err(1, "%s", devstat_errbuf);
		else if (retval == 1)
			return(2);
	}
	return(1);
}