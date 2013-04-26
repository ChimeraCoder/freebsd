
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

#define MAGIC 123456789
struct my_data {
	int magic;
	int read_return;
	int read_called;
	int write_return;
	int write_called;
	int open_return;
	int open_called;
	int close_return;
	int close_called;
};

static ssize_t
my_read(struct archive *a, void *_private, const void **buff)
{
	struct my_data *private = (struct my_data *)_private;
	(void)a; /* UNUSED */
	(void)buff; /* UNUSED */
	assertEqualInt(MAGIC, private->magic);
	++private->read_called;
	return (private->read_return);
}

static ssize_t
my_write(struct archive *a, void *_private, const void *buff, size_t s)
{
	struct my_data *private = (struct my_data *)_private;
	(void)a; /* UNUSED */
	(void)buff; /* UNUSED */
	(void)s; /* UNUSED */
	assertEqualInt(MAGIC, private->magic);
	++private->write_called;
	return (private->write_return);
}

static int
my_open(struct archive *a, void *_private)
{
	struct my_data *private = (struct my_data *)_private;
	(void)a; /* UNUSED */
	assertEqualInt(MAGIC, private->magic);
	++private->open_called;
	return (private->open_return);
}

static int
my_close(struct archive *a, void *_private)
{
	struct my_data *private = (struct my_data *)_private;
	(void)a; /* UNUSED */
	assertEqualInt(MAGIC, private->magic);
	++private->close_called;
	return (private->close_return);
}


DEFINE_TEST(test_open_failure)
{
	struct archive *a;
	struct my_data private;

	memset(&private, 0, sizeof(private));
	private.magic = MAGIC;
	private.open_return = ARCHIVE_FATAL;
	a = archive_read_new();
	assert(a != NULL);
	assertEqualInt(ARCHIVE_FATAL,
	    archive_read_open(a, &private, my_open, my_read, my_close));
	assertEqualInt(1, private.open_called);
	assertEqualInt(0, private.read_called);
	assertEqualInt(1, private.close_called);
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
	assertEqualInt(1, private.open_called);
	assertEqualInt(0, private.read_called);
	assertEqualInt(1, private.close_called);

	memset(&private, 0, sizeof(private));
	private.magic = MAGIC;
	private.open_return = ARCHIVE_FAILED;
	a = archive_read_new();
	assert(a != NULL);
	assertEqualInt(ARCHIVE_FAILED,
	    archive_read_open(a, &private, my_open, my_read, my_close));
	assertEqualInt(1, private.open_called);
	assertEqualInt(0, private.read_called);
	assertEqualInt(1, private.close_called);
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
	assertEqualInt(1, private.open_called);
	assertEqualInt(0, private.read_called);
	assertEqualInt(1, private.close_called);

	memset(&private, 0, sizeof(private));
	private.magic = MAGIC;
	private.open_return = ARCHIVE_WARN;
	a = archive_read_new();
	assert(a != NULL);
	assertEqualInt(ARCHIVE_WARN,
	    archive_read_open(a, &private, my_open, my_read, my_close));
	assertEqualInt(1, private.open_called);
	assertEqualInt(0, private.read_called);
	assertEqualInt(1, private.close_called);
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
	assertEqualInt(1, private.open_called);
	assertEqualInt(0, private.read_called);
	assertEqualInt(1, private.close_called);

	memset(&private, 0, sizeof(private));
	private.magic = MAGIC;
	private.open_return = ARCHIVE_OK;
	private.read_return = ARCHIVE_FATAL;
	a = archive_read_new();
	assert(a != NULL);
	assertEqualInt(ARCHIVE_OK,
	    archive_read_support_filter_compress(a));
	assertEqualInt(ARCHIVE_OK, archive_read_support_format_tar(a));
	assertEqualInt(ARCHIVE_FATAL,
	    archive_read_open(a, &private, my_open, my_read, my_close));
	assertEqualInt(1, private.open_called);
	assertEqualInt(1, private.read_called);
	assertEqualInt(1, private.close_called);
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
	assertEqualInt(1, private.open_called);
	assertEqualInt(1, private.read_called);
	assertEqualInt(1, private.close_called);

	memset(&private, 0, sizeof(private));
	private.magic = MAGIC;
	private.open_return = ARCHIVE_FATAL;
	a = archive_write_new();
	assert(a != NULL);
	assertEqualInt(ARCHIVE_FATAL,
	    archive_write_open(a, &private, my_open, my_write, my_close));
	assertEqualInt(1, private.open_called);
	assertEqualInt(0, private.write_called);
	assertEqualInt(1, private.close_called);
	assertEqualInt(ARCHIVE_OK, archive_write_free(a));
	assertEqualInt(1, private.open_called);
	assertEqualInt(0, private.write_called);
	assertEqualInt(1, private.close_called);

	memset(&private, 0, sizeof(private));
	private.magic = MAGIC;
	private.open_return = ARCHIVE_FATAL;
	a = archive_write_new();
	assert(a != NULL);
	archive_write_add_filter_compress(a);
	archive_write_set_format_ustar(a);
	assertEqualInt(ARCHIVE_FATAL,
	    archive_write_open(a, &private, my_open, my_write, my_close));
	assertEqualInt(1, private.open_called);
	assertEqualInt(0, private.write_called);
	assertEqualInt(1, private.close_called);
	assertEqualInt(ARCHIVE_OK, archive_write_free(a));
	assertEqualInt(1, private.open_called);
	assertEqualInt(0, private.write_called);
	assertEqualInt(1, private.close_called);

	memset(&private, 0, sizeof(private));
	private.magic = MAGIC;
	private.open_return = ARCHIVE_FATAL;
	a = archive_write_new();
	assert(a != NULL);
	archive_write_set_format_zip(a);
	assertEqualInt(ARCHIVE_FATAL,
	    archive_write_open(a, &private, my_open, my_write, my_close));
	assertEqualInt(1, private.open_called);
	assertEqualInt(0, private.write_called);
	assertEqualInt(1, private.close_called);
	assertEqualInt(ARCHIVE_OK, archive_write_free(a));
	assertEqualInt(1, private.open_called);
	assertEqualInt(0, private.write_called);
	assertEqualInt(1, private.close_called);

	memset(&private, 0, sizeof(private));
	private.magic = MAGIC;
	private.open_return = ARCHIVE_FATAL;
	a = archive_write_new();
	assert(a != NULL);
	archive_write_add_filter_gzip(a);
	assertEqualInt(ARCHIVE_FATAL,
	    archive_write_open(a, &private, my_open, my_write, my_close));
	assertEqualInt(1, private.open_called);
	assertEqualInt(0, private.write_called);
	assertEqualInt(1, private.close_called);
	assertEqualInt(ARCHIVE_OK, archive_write_free(a));
	assertEqualInt(1, private.open_called);
	assertEqualInt(0, private.write_called);
	assertEqualInt(1, private.close_called);

}