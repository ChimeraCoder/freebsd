
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

static const char *
gname_lookup(void *d, int64_t g)
{
	(void)d; /* UNUSED */
	(void)g; /* UNUSED */
	return ("FOOGROUP");
}

static const char *
uname_lookup(void *d, int64_t u)
{
	(void)d; /* UNUSED */
	(void)u; /* UNUSED */
	return ("FOO");
}

DEFINE_TEST(test_read_disk_entry_from_file)
{
	struct archive *a;
	struct archive_entry *entry;
	FILE *f;

	assert((a = archive_read_disk_new()) != NULL);

	assertEqualInt(ARCHIVE_OK, archive_read_disk_set_uname_lookup(a,
			   NULL, &uname_lookup, NULL));
	assertEqualInt(ARCHIVE_OK, archive_read_disk_set_gname_lookup(a,
			   NULL, &gname_lookup, NULL));
	assertEqualString(archive_read_disk_uname(a, 0), "FOO");
	assertEqualString(archive_read_disk_gname(a, 0), "FOOGROUP");

	/* Create a file on disk. */
	f = fopen("foo", "wb");
	assert(f != NULL);
	assertEqualInt(4, fwrite("1234", 1, 4, f));
	fclose(f);

	/* Use archive_read_disk_entry_from_file to get information about it. */
	entry = archive_entry_new();
	assert(entry != NULL);
	archive_entry_copy_pathname(entry, "foo");
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_disk_entry_from_file(a, entry, -1, NULL));

	/* Verify the information we got back. */
	assertEqualString(archive_entry_uname(entry), "FOO");
	assertEqualString(archive_entry_gname(entry), "FOOGROUP");
	assertEqualInt(archive_entry_size(entry), 4);

	/* Destroy the archive. */
	archive_entry_free(entry);
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}