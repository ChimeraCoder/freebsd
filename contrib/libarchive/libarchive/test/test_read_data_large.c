
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

/*
 * Test read/write of a 10M block of data in a single operation.
 * Uses an in-memory archive with a single 10M entry.  Exercises
 * archive_read_data() to ensure it can handle large blocks like
 * this and also exercises archive_read_data_into_fd() (which
 * had a bug relating to this, fixed in Nov 2006).
 */

#if defined(_WIN32) && !defined(__CYGWIN__)
#define open _open
#define close _close
#endif

char buff1[11000000];
char buff2[10000000];
char buff3[10000000];

DEFINE_TEST(test_read_data_large)
{
	struct archive_entry *ae;
	struct archive *a;
	char tmpfilename[] = "largefile";
	int tmpfilefd;
	FILE *f;
	unsigned int i;
	size_t used;

	/* Create a new archive in memory. */
	assert((a = archive_write_new()) != NULL);
	assertA(0 == archive_write_set_format_ustar(a));
	assertA(0 == archive_write_add_filter_none(a));
	assertA(0 == archive_write_open_memory(a, buff1, sizeof(buff1), &used));

	/*
	 * Write a file (with random contents) to it.
	 */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "file");
	archive_entry_set_mode(ae, S_IFREG | 0755);
	for (i = 0; i < sizeof(buff2); i++)
		buff2[i] = (unsigned char)rand();
	archive_entry_set_size(ae, sizeof(buff2));
	assertA(0 == archive_write_header(a, ae));
	archive_entry_free(ae);
	assertA((int)sizeof(buff2) == archive_write_data(a, buff2, sizeof(buff2)));

	/* Close out the archive. */
	assertEqualIntA(a, ARCHIVE_OK, archive_write_close(a));
	assertEqualInt(ARCHIVE_OK, archive_write_free(a));

	/* Check that archive_read_data can handle 10*10^6 at a pop. */
	assert((a = archive_read_new()) != NULL);
	assertA(0 == archive_read_support_format_all(a));
	assertA(0 == archive_read_support_filter_all(a));
	assertA(0 == archive_read_open_memory(a, buff1, sizeof(buff1)));
	assertA(0 == archive_read_next_header(a, &ae));
	failure("Wrote 10MB, but didn't read the same amount");
	assertEqualIntA(a, sizeof(buff2),archive_read_data(a, buff3, sizeof(buff3)));
	failure("Read expected 10MB, but data read didn't match what was written");
	assertEqualMem(buff2, buff3, sizeof(buff3));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));

	/* Check archive_read_data_into_fd */
	assert((a = archive_read_new()) != NULL);
	assertA(0 == archive_read_support_format_all(a));
	assertA(0 == archive_read_support_filter_all(a));
	assertA(0 == archive_read_open_memory(a, buff1, sizeof(buff1)));
	assertA(0 == archive_read_next_header(a, &ae));
#if defined(__BORLANDC__)
	tmpfilefd = open(tmpfilename, O_WRONLY | O_CREAT | O_BINARY);
#else
	tmpfilefd = open(tmpfilename, O_WRONLY | O_CREAT | O_BINARY, 0777);
#endif
	assert(tmpfilefd != 0);
	assertEqualIntA(a, 0, archive_read_data_into_fd(a, tmpfilefd));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
	close(tmpfilefd);

	f = fopen(tmpfilename, "rb");
	assert(f != NULL);
	assertEqualInt(sizeof(buff3), fread(buff3, 1, sizeof(buff3), f));
	fclose(f);
	assertEqualMem(buff2, buff3, sizeof(buff3));
}