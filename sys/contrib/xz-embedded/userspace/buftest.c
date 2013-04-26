
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
 * Test application to test buffer-to-buffer decoding
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "xz.h"

#define BUFFER_SIZE (1024 * 1024)

static uint8_t in[BUFFER_SIZE];
static uint8_t out[BUFFER_SIZE];

int main(void)
{
	struct xz_buf b;
	struct xz_dec *s;
	enum xz_ret ret;

	xz_crc32_init();

	s = xz_dec_init(XZ_SINGLE, 0);
	if (s == NULL) {
		fputs("Initialization failed", stderr);
		return 1;
	}

	b.in = in;
	b.in_pos = 0;
	b.in_size = fread(in, 1, sizeof(in), stdin);
	b.out = out;
	b.out_pos = 0;
	b.out_size = sizeof(out);

	ret = xz_dec_run(s, &b);
	xz_dec_end(s);

	fwrite(out, 1, b.out_pos, stdout);
	fprintf(stderr, "%d\n", ret);

	return 0;
}