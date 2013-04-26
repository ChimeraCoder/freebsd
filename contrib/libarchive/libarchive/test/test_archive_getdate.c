
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
#include "test.h"
__FBSDID("$FreeBSD$");

#include <time.h>

/*
 * Verify that the getdate() function works.
 */

time_t __archive_get_date(time_t, const char *);
#define get_date __archive_get_date

DEFINE_TEST(test_archive_getdate)
{
	time_t now = time(NULL);

	assertEqualInt(get_date(now, "Jan 1, 1970 UTC"), 0);
	assertEqualInt(get_date(now, "7:12:18-0530 4 May 1983"), 420900138);
	assertEqualInt(get_date(now, "2004/01/29 513 mest"), 1075345980);
	assertEqualInt(get_date(now, "99/02/17 7pm utc"), 919278000);
	assertEqualInt(get_date(now, "02/17/99 7:11am est"), 919253460);
	assertEqualInt(get_date(now, "now - 2 hours"),
	    get_date(now, "2 hours ago"));
	/* It's important that we handle ctime() format. */
	assertEqualInt(get_date(now, "Sun Feb 22 17:38:26 PST 2009"),
	    1235353106);
	/* Basic relative offsets. */
	/* If we use the actual current time as the reference, then
	 * these tests break around DST changes, so it's actually
	 * important to use a specific reference time here. */
	assertEqualInt(get_date(0, "tomorrow"), 24 * 60 * 60);
	assertEqualInt(get_date(0, "yesterday"), - 24 * 60 * 60);
	assertEqualInt(get_date(0, "now + 1 hour"), 60 * 60);
	assertEqualInt(get_date(0, "now + 1 hour + 1 minute"), 60 * 60 + 60);
	/* Repeat the above for a different start time. */
	now = 1231113600; /* Jan 5, 2009 00:00 UTC */
	assertEqualInt(get_date(0, "Jan 5, 2009 00:00 UTC"), now);
	assertEqualInt(get_date(now, "tomorrow"), now + 24 * 60 * 60);
	assertEqualInt(get_date(now, "yesterday"), now - 24 * 60 * 60);
	assertEqualInt(get_date(now, "now + 1 hour"), now + 60 * 60);
	assertEqualInt(get_date(now, "now + 1 hour + 1 minute"),
	    now + 60 * 60 + 60);
	assertEqualInt(get_date(now, "tomorrow 5:16am UTC"),
	    now + 24 * 60 * 60 + 5 * 60 * 60 + 16 * 60);
	assertEqualInt(get_date(now, "UTC 5:16am tomorrow"),
	    now + 24 * 60 * 60 + 5 * 60 * 60 + 16 * 60);

	/* Jan 5, 2009 was a Monday. */
	assertEqualInt(get_date(now, "monday UTC"), now);
	assertEqualInt(get_date(now, "sunday UTC"), now + 6 * 24 * 60 * 60);
	assertEqualInt(get_date(now, "tuesday UTC"), now + 24 * 60 * 60);
	/* "next tuesday" is one week after "tuesday" */
	assertEqualInt(get_date(now, "UTC next tuesday"),
	    now + 8 * 24 * 60 * 60);
	/* "last tuesday" is one week before "tuesday" */
	assertEqualInt(get_date(now, "last tuesday UTC"),
	    now - 6 * 24 * 60 * 60);

	/* TODO: Lots more tests here. */
}