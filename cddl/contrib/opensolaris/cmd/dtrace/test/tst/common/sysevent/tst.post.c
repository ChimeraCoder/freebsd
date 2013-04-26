
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

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <libsysevent.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
	sysevent_id_t id;

	for (;;) {
		if (sysevent_post_event("class_dtest", "subclass_dtest",
		    "vendor_dtest", "publisher_dtest", NULL, &id) != 0) {
			(void) fprintf(stderr, "failed to post sysevent\n");
			return (1);
		}

		sleep(1);
	}
}