
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
/*
 * Test application for xz_boot.c
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define STATIC static
#define INIT

static void error(/*const*/ char *msg)
{
	fprintf(stderr, "%s\n", msg);
}

#include "../linux/lib/decompress_unxz.c"

static uint8_t in[1024 * 1024];
static uint8_t out[1024 * 1024];

static int fill(void *buf, unsigned int size)
{
	return fread(buf, 1, size, stdin);
}

static int flush(/*const*/ void *buf, unsigned int size)
{
	return fwrite(buf, 1, size, stdout);
}

static void test_buf_to_buf(void)
{
	size_t in_size;
	int ret;
	in_size = fread(in, 1, sizeof(in), stdin);
	ret = decompress(in, in_size, NULL, NULL, out, NULL, &error);
	/* fwrite(out, 1, FIXME, stdout); */
	fprintf(stderr, "ret = %d\n", ret);
}

static void test_buf_to_cb(void)
{
	size_t in_size;
	int in_used;
	int ret;
	in_size = fread(in, 1, sizeof(in), stdin);
	ret = decompress(in, in_size, NULL, &flush, NULL, &in_used, &error);
	fprintf(stderr, "ret = %d; in_used = %d\n", ret, in_used);
}

static void test_cb_to_cb(void)
{
	int ret;
	ret = decompress(NULL, 0, &fill, &flush, NULL, NULL, &error);
	fprintf(stderr, "ret = %d\n", ret);
}

/*
 * Not used by Linux <= 2.6.37-rc4 and newer probably won't use it either,
 * but this kind of use case is still required to be supported by the API.
 */
static void test_cb_to_buf(void)
{
	int in_used;
	int ret;
	ret = decompress(in, 0, &fill, NULL, out, &in_used, &error);
	/* fwrite(out, 1, FIXME, stdout); */
	fprintf(stderr, "ret = %d; in_used = %d\n", ret, in_used);
}

int main(int argc, char **argv)
{
	if (argc != 2)
		fprintf(stderr, "Usage: %s [bb|bc|cc|cb]\n", argv[0]);
	else if (strcmp(argv[1], "bb") == 0)
		test_buf_to_buf();
	else if (strcmp(argv[1], "bc") == 0)
		test_buf_to_cb();
	else if (strcmp(argv[1], "cc") == 0)
		test_cb_to_cb();
	else if (strcmp(argv[1], "cb") == 0)
		test_cb_to_buf();
	else
		fprintf(stderr, "Usage: %s [bb|bc|cc|cb]\n", argv[0]);

	return 0;
}