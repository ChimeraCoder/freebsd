
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

#include <sys/types.h>
#include <errno.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libutil.h>


/*
 * Static values for building and testing an artificial group.
 */
static char grpName[] = "groupName";
static char grpPasswd[] = "groupPwd";
static gid_t grpGID = 1234;
static char *grpMems[] = { "mem1", "mem2", "mem3", NULL };
static const char *origStrGrp = "groupName:groupPwd:1234:mem1,mem2,mem3";


/*
 * Build a group to test against without depending on a real group to be found
 * within /etc/group.
 */
static void
build_grp(struct group *grp)
{
	grp->gr_name = grpName;
	grp->gr_passwd = grpPasswd;
	grp->gr_gid = grpGID;
	grp->gr_mem = grpMems;

	return;
}


int
main(void)
{
	char *strGrp;
	int testNdx;
	struct group *dupGrp;
	struct group *scanGrp;
	struct group origGrp;

	/* Setup. */
	printf("1..4\n");
	testNdx = 0;

	/* Manually build a group using static values. */
	build_grp(&origGrp);

	/* Copy the group. */
	testNdx++;
	if ((dupGrp = gr_dup(&origGrp)) == NULL)
		printf("not ");
	printf("ok %d - %s\n", testNdx, "gr_dup");

	/* Compare the original and duplicate groups. */
	testNdx++;
	if (! gr_equal(&origGrp, dupGrp))
		printf("not ");
	printf("ok %d - %s\n", testNdx, "gr_equal");

	/* Create group string from the duplicate group structure. */
	testNdx++;
	strGrp = gr_make(dupGrp);
	if (strcmp(strGrp, origStrGrp) != 0)
		printf("not ");
	printf("ok %d - %s\n", testNdx, "gr_make");

	/*
	 * Create group structure from string and compare it to the original
	 * group structure.
	 */
	testNdx++;
	if ((scanGrp = gr_scan(strGrp)) == NULL || ! gr_equal(&origGrp,
	    scanGrp))
		printf("not ");
	printf("ok %d - %s\n", testNdx, "gr_scan");

	/* Clean up. */
	free(scanGrp);
	free(strGrp);
	free(dupGrp);

	exit(EXIT_SUCCESS);
}