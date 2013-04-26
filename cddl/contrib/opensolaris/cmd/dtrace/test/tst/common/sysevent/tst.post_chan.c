
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

#include <libsysevent.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
	evchan_t *ch;

	if (sysevent_evc_bind("channel_dtest", &ch,
	    EVCH_CREAT | EVCH_HOLD_PEND) != 0) {
		(void) fprintf(stderr, "failed to bind to sysevent channel\n");
		return (1);
	}

	for (;;) {
		if (sysevent_evc_publish(ch, "class_dtest", "subclass_dtest",
		    "vendor_dtest", "publisher_dtest", NULL, EVCH_SLEEP) != 0) {
			(void) sysevent_evc_unbind(ch);
			(void) fprintf(stderr, "failed to publisth sysevent\n");
			return (1);
		}
		sleep(1);
	}
}