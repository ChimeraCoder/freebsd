
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <security/pam_appl.h>
#include <security/openpam.h>

#include "openpam_impl.h"
#include "t.h"

static char filename[1024];
static FILE *f;

/*
 * Open the temp file and immediately unlink it so it doesn't leak in case
 * of premature exit.
 */
static void
orlv_open(void)
{
	int fd;

	if ((fd = open(filename, O_RDWR|O_CREAT|O_TRUNC, 0600)) < 0)
		err(1, "%s(): %s", __func__, filename);
	if ((f = fdopen(fd, "r+")) == NULL)
		err(1, "%s(): %s", __func__, filename);
	if (unlink(filename) < 0)
		err(1, "%s(): %s", __func__, filename);
}

/*
 * Write text to the temp file.
 */
static void
orlv_output(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(f, fmt, ap);
	va_end(ap);
	if (ferror(f))
		err(1, "%s", filename);
}

/*
 * Rewind the temp file.
 */
static void
orlv_rewind(void)
{

	errno = 0;
	rewind(f);
	if (errno != 0)
		err(1, "%s(): %s", __func__, filename);
}

/*
 * Read a line from the temp file and verify that the result matches our
 * expectations: whether a line was read at all, how many and which words
 * it contained, how many lines were read (in case of quoted or escaped
 * newlines) and whether we reached the end of the file.
 */
static int
orlv_expect(const char **expectedv, int lines, int eof)
{
	int expectedc, gotc, i, lineno = 0;
	char **gotv;

	expectedc = 0;
	if (expectedv != NULL)
		while (expectedv[expectedc] != NULL)
			++expectedc;
	gotv = openpam_readlinev(f, &lineno, &gotc);
	if (ferror(f))
		err(1, "%s(): %s", __func__, filename);
	if (expectedv != NULL && gotv == NULL) {
		t_verbose("expected %d words, got nothing\n", expectedc);
		return (0);
	}
	if (expectedv == NULL && gotv != NULL) {
		t_verbose("expected nothing, got %d words\n", gotc);
		FREEV(gotc, gotv);
		return (0);
	}
	if (expectedv != NULL && gotv != NULL) {
		if (expectedc != gotc) {
			t_verbose("expected %d words, got %d\n",
			    expectedc, gotc);
			FREEV(gotc, gotv);
			return (0);
		}
		for (i = 0; i < gotc; ++i) {
			if (strcmp(expectedv[i], gotv[i]) != 0) {
				t_verbose("word %d: expected <<%s>>, "
				    "got <<%s>>\n", i, expectedv[i], gotv[i]);
				FREEV(gotc, gotv);
				return (0);
			}
		}
		FREEV(gotc, gotv);
	}
	if (lineno != lines) {
		t_verbose("expected to advance %d lines, advanced %d lines\n",
		    lines, lineno);
		return (0);
	}
	if (eof && !feof(f)) {
		t_verbose("expected EOF, but didn't get it\n");
		return (0);
	}
	if (!eof && feof(f)) {
		t_verbose("didn't expect EOF, but got it anyway\n");
		return (0);
	}
	return (1);
}

/*
 * Close the temp file.
 */
void
orlv_close(void)
{

	if (fclose(f) != 0)
		err(1, "%s(): %s", __func__, filename);
	f = NULL;
}

/***************************************************************************
 * Commonly-used lines
 */

static const char *empty[] = {
	NULL
};

static const char *hello[] = {
	"hello",
	NULL
};

static const char *hello_world[] = {
	"hello",
	"world",
	NULL
};


/***************************************************************************
 * Lines without words
 */

T_FUNC(empty_input, "empty input")
{
	int ret;

	orlv_open();
	ret = orlv_expect(NULL, 0 /*lines*/, 1 /*eof*/);
	orlv_close();
	return (ret);
}

T_FUNC(empty_line, "empty line")
{
	int ret;

	orlv_open();
	orlv_output("\n");
	orlv_rewind();
	ret = orlv_expect(empty, 1 /*lines*/, 0 /*eof*/);
	orlv_close();
	return (ret);
}

T_FUNC(unterminated_empty_line, "unterminated empty line")
{
	int ret;

	orlv_open();
	orlv_output(" ");
	orlv_rewind();
	ret = orlv_expect(NULL, 0 /*lines*/, 1 /*eof*/);
	orlv_close();
	return (ret);
}

T_FUNC(whitespace, "whitespace")
{
	int ret;

	orlv_open();
	orlv_output(" \n");
	orlv_rewind();
	ret = orlv_expect(empty, 1 /*lines*/, 0 /*eof*/);
	orlv_close();
	return (ret);
}

T_FUNC(comment, "comment")
{
	int ret;

	orlv_open();
	orlv_output("# comment\n");
	orlv_rewind();
	ret = orlv_expect(empty, 1 /*lines*/, 0 /*eof*/);
	orlv_close();
	return (ret);
}

T_FUNC(whitespace_before_comment, "whitespace before comment")
{
	int ret;

	orlv_open();
	orlv_output(" # comment\n");
	orlv_rewind();
	ret = orlv_expect(empty, 1 /*lines*/, 0 /*eof*/);
	orlv_close();
	return (ret);
}


/***************************************************************************
 * Simple words
 */

T_FUNC(one_word, "one word")
{
	int ret;

	orlv_open();
	orlv_output("hello\n");
	orlv_rewind();
	ret = orlv_expect(hello, 1 /*lines*/, 0 /*eof*/);
	orlv_close();
	return (ret);
}

T_FUNC(two_words, "two words")
{
	int ret;

	orlv_open();
	orlv_output("hello world\n");
	orlv_rewind();
	ret = orlv_expect(hello_world, 1 /*lines*/, 0 /*eof*/);
	orlv_close();
	return (ret);
}

T_FUNC(unterminated_line, "unterminated line")
{
	int ret;

	orlv_open();
	orlv_output("hello world");
	orlv_rewind();
	ret = orlv_expect(hello_world, 0 /*lines*/, 1 /*eof*/);
	orlv_close();
	return (ret);
}


/***************************************************************************
 * Boilerplate
 */

const struct t_test *t_plan[] = {
	T(empty_input),
	T(empty_line),
	T(unterminated_empty_line),
	T(whitespace),
	T(comment),
	T(whitespace_before_comment),

	T(one_word),
	T(two_words),
	T(unterminated_line),

	NULL
};

const struct t_test **
t_prepare(int argc, char *argv[])
{

	(void)argc;
	(void)argv;
	snprintf(filename, sizeof filename, "%s.%d.tmp", t_progname, getpid());
	if (filename == NULL)
		err(1, "asprintf()");
	return (t_plan);
}

void
t_cleanup(void)
{
}