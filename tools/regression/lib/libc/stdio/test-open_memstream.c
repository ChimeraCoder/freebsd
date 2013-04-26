
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

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static char *buf;
static size_t len;

static void
assert_stream(const char *contents)
{
	if (strlen(contents) != len)
		printf("bad length %zd for \"%s\"\n", len, contents);
	else if (strncmp(buf, contents, strlen(contents)) != 0)
		printf("bad buffer \"%s\" for \"%s\"\n", buf, contents);
}

static void
open_group_test(void)
{
	FILE *fp;
	off_t eob;

	fp = open_memstream(&buf, &len);
	if (fp == NULL)
		err(1, "failed to open stream");

	fprintf(fp, "hello my world");
	fflush(fp);
	assert_stream("hello my world");
	eob = ftello(fp);
	rewind(fp);
	fprintf(fp, "good-bye");
	fseeko(fp, eob, SEEK_SET);
	fclose(fp);
	assert_stream("good-bye world");
	free(buf);
}

static void
simple_tests(void)
{
	static const char zerobuf[] =
	    { 'f', 'o', 'o', 0, 0, 0, 0, 'b', 'a', 'r', 0 };
	char c;
	FILE *fp;

	fp = open_memstream(&buf, NULL);
	if (fp != NULL)
		errx(1, "did not fail to open stream");
	else if (errno != EINVAL)
		err(1, "incorrect error for bad length pointer");
	fp = open_memstream(NULL, &len);
	if (fp != NULL)
		errx(1, "did not fail to open stream");
	else if (errno != EINVAL)
		err(1, "incorrect error for bad buffer pointer");
	fp = open_memstream(&buf, &len);
	if (fp == NULL)
		err(1, "failed to open stream");
	fflush(fp);
	assert_stream("");
	if (fwide(fp, 0) >= 0)
		printf("stream is not byte-oriented\n");

	fprintf(fp, "fo");
	fflush(fp);
	assert_stream("fo");
	fputc('o', fp);
	fflush(fp);
	assert_stream("foo");
	rewind(fp);
	fflush(fp);
	assert_stream("");
	fseek(fp, 0, SEEK_END);
	fflush(fp);
	assert_stream("foo");

	/*
	 * Test seeking out past the current end.  Should zero-fill the
	 * intermediate area.
	 */
	fseek(fp, 4, SEEK_END);
	fprintf(fp, "bar");
	fflush(fp);

	/*
	 * Can't use assert_stream() here since this should contain
	 * embedded null characters.
	 */
	if (len != 10)
		printf("bad length %zd for zero-fill test\n", len);
	else if (memcmp(buf, zerobuf, sizeof(zerobuf)) != 0)
		printf("bad buffer for zero-fill test\n");

	fseek(fp, 3, SEEK_SET);
	fprintf(fp, " in ");
	fflush(fp);
	assert_stream("foo in ");
	fseek(fp, 0, SEEK_END);
	fflush(fp);
	assert_stream("foo in bar");

	rewind(fp);
	if (fread(&c, sizeof(c), 1, fp) != 0)
		printf("fread did not fail\n");
	else if (!ferror(fp))
		printf("error indicator not set after fread\n");
	else
		clearerr(fp);

	fseek(fp, 4, SEEK_SET);
	fprintf(fp, "bar baz");
	fclose(fp);
	assert_stream("foo bar baz");
	free(buf);
}

static void
seek_tests(void)
{
	FILE *fp;

	fp = open_memstream(&buf, &len);
	if (fp == NULL)
		err(1, "failed to open stream");
#define SEEK_FAIL(offset, whence, error) do {				\
	errno = 0;							\
	if (fseeko(fp, (offset), (whence)) == 0)			\
		printf("fseeko(%s, %s) did not fail, set pos to %jd\n",	\
		    __STRING(offset), __STRING(whence),			\
		    (intmax_t)ftello(fp));				\
	else if (errno != (error))					\
		printf("fseeko(%s, %s) failed with %d rather than %s\n",\
		    __STRING(offset), __STRING(whence),	errno,		\
		    __STRING(error));					\
} while (0)

#define SEEK_OK(offset, whence, result) do {				\
	if (fseeko(fp, (offset), (whence)) != 0)			\
		printf("fseeko(%s, %s) failed: %s\n",			\
		    __STRING(offset), __STRING(whence),	strerror(errno)); \
	else if (ftello(fp) != (result))				\
		printf("fseeko(%s, %s) seeked to %jd rather than %s\n",	\
		    __STRING(offset), __STRING(whence),			\
		    (intmax_t)ftello(fp), __STRING(result));		\
} while (0)

	SEEK_FAIL(-1, SEEK_SET, EINVAL);
	SEEK_FAIL(-1, SEEK_CUR, EINVAL);
	SEEK_FAIL(-1, SEEK_END, EINVAL);
	fprintf(fp, "foo");
	SEEK_OK(-1, SEEK_CUR, 2);
	SEEK_OK(0, SEEK_SET, 0);
	SEEK_OK(-1, SEEK_END, 2);
	SEEK_OK(OFF_MAX - 1, SEEK_SET, OFF_MAX - 1);
	SEEK_FAIL(2, SEEK_CUR, EOVERFLOW);
	fclose(fp);
}

int
main(int ac, char **av)
{

	open_group_test();
	simple_tests();
	seek_tests();
	return (0);
}