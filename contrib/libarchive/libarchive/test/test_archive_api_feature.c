
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

DEFINE_TEST(test_archive_api_feature)
{
	char buff[128];
	const char *p;

	/* This is the (hopefully) final versioning API. */
	assertEqualInt(ARCHIVE_VERSION_NUMBER, archive_version_number());
	sprintf(buff, "libarchive %d.%d.%d",
	    archive_version_number() / 1000000,
	    (archive_version_number() / 1000) % 1000,
	    archive_version_number() % 1000);
	failure("Version string is: %s, computed is: %s",
	    archive_version_string(), buff);
	assertEqualMem(buff, archive_version_string(), strlen(buff));
	if (strlen(buff) < strlen(archive_version_string())) {
		p = archive_version_string() + strlen(buff);
		failure("Version string is: %s", archive_version_string());
		assert(*p == 'a' || *p == 'b' || *p == 'c' || *p == 'd');
		++p;
		failure("Version string is: %s", archive_version_string());
		assert(*p == '\0');
	}
}