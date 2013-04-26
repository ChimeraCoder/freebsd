
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
ecute the following to rebuild the data for this program:
   tail -n +33 test_read_format_cpio_afio.c | /bin/sh

# How to make a sample data.
echo "0123456789abcdef" > file1
echo "0123456789abcdef" > file2
# make afio use a large ASCII header
sudo chown 65536 file2
find . -name "file[12]" | afio -o sample
od -c sample | sed -E -e "s/^0[0-9]+//;s/^  //;s/( +)([^ ]{1,2})/'\2',/g;s/'\\0'/0/g;/^[*]/d" > test_read_format_cpio_afio.sample.txt
rm -f file1 file2 sample
exit1
*/

static unsigned char archive[] = {
'0','7','0','7','0','7','0','0','0','1','4','3','1','2','5','3',
'2','1','1','0','0','6','4','4','0','0','1','7','5','1','0','0',
'1','7','5','1','0','0','0','0','0','1','0','0','0','0','0','0',
'1','1','3','3','2','2','4','5','0','2','0','0','0','0','0','0',
'6','0','0','0','0','0','0','0','0','0','2','1','f','i','l','e',
'1',0,'0','1','2','3','4','5','6','7','8','9','a','b','c','d',
'e','f','\n','0','7','0','7','2','7','0','0','0','0','0','0','6',
'3','0','0','0','0','0','0','0','0','0','0','0','D','A','A','E',
'6','m','1','0','0','6','4','4','0','0','0','1','0','0','0','0',
'0','0','0','0','0','3','E','9','0','0','0','0','0','0','0','1',
'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
'4','B','6','9','4','A','1','0','n','0','0','0','6','0','0','0',
'0','0','0','0','0','s','0','0','0','0','0','0','0','0','0','0',
'0','0','0','0','1','1',':','f','i','l','e','2',0,'0','1','2',
'3','4','5','6','7','8','9','a','b','c','d','e','f','\n','0','7',
'0','7','0','7','0','0','0','0','0','0','0','0','0','0','0','0',
'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
'0','0','0','0','0','0','0','1','0','0','0','0','0','0','0','0',
'0','0','0','0','0','0','0','0','0','0','0','0','0','1','3','0',
'0','0','0','0','0','1','1','2','7','3','T','R','A','I','L','E',
'R','!','!','!',0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

/*
 * XXX This must be removed when we use int64_t for uid.
 */
static int
uid_size(void)
{
	return (sizeof(uid_t));
}

DEFINE_TEST(test_read_format_cpio_afio)
{
	unsigned char *p;
	size_t size;
	struct archive_entry *ae;
	struct archive *a;

	/* The default block size of afio is 5120. we simulate it */
	size = (sizeof(archive) + 5120 -1 / 5120) * 5120;
	assert((p = malloc(size)) != NULL);
	if (p == NULL)
		return;
	memset(p, 0, size);
	memcpy(p, archive, sizeof(archive));
	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_open_memory(a, p, size));
	/*
	 * First entry is odc format.
	 */
	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualInt(17, archive_entry_size(ae));
	assertA(archive_filter_code(a, 0) == ARCHIVE_FILTER_NONE);
	assertA(archive_format(a) == ARCHIVE_FORMAT_CPIO_POSIX);
	/*
	 * Second entry is afio large ASCII format.
	 */
	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualInt(17, archive_entry_size(ae));
	if (uid_size() > 4)
		assertEqualInt(65536, archive_entry_uid(ae));
	assertA(archive_filter_code(a, 0) == ARCHIVE_FILTER_NONE);
	assertA(archive_format(a) == ARCHIVE_FORMAT_CPIO_AFIO_LARGE);
	assertEqualInt(ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));

	free(p);
}