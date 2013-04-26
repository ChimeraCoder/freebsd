
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

#include "includes.h"

#include <sys/types.h>
#include <netinet/in.h>
#include <resolv.h>
#include <stdio.h>

#include "xmalloc.h"
#include "uuencode.h"

/*
 * Encode binary 'src' of length 'srclength', writing base64-encoded text
 * to 'target' of size 'targsize'. Will always nul-terminate 'target'.
 * Returns the number of bytes stored in 'target' or -1 on error (inc.
 * 'targsize' too small).
 */
int
uuencode(const u_char *src, u_int srclength,
    char *target, size_t targsize)
{
	return __b64_ntop(src, srclength, target, targsize);
}

/*
 * Decode base64-encoded 'src' into buffer 'target' of 'targsize' bytes.
 * Will skip leading and trailing whitespace. Returns the number of bytes
 * stored in 'target' or -1 on error (inc. targsize too small).
 */
int
uudecode(const char *src, u_char *target, size_t targsize)
{
	int len;
	char *encoded, *p;

	/* copy the 'readonly' source */
	encoded = xstrdup(src);
	/* skip whitespace and data */
	for (p = encoded; *p == ' ' || *p == '\t'; p++)
		;
	for (; *p != '\0' && *p != ' ' && *p != '\t'; p++)
		;
	/* and remove trailing whitespace because __b64_pton needs this */
	*p = '\0';
	len = __b64_pton(encoded, target, targsize);
	xfree(encoded);
	return len;
}

void
dump_base64(FILE *fp, const u_char *data, u_int len)
{
	char *buf;
	int i, n;

	if (len > 65536) {
		fprintf(fp, "dump_base64: len > 65536\n");
		return;
	}
	buf = xmalloc(2*len);
	n = uuencode(data, len, buf, 2*len);
	for (i = 0; i < n; i++) {
		fprintf(fp, "%c", buf[i]);
		if (i % 70 == 69)
			fprintf(fp, "\n");
	}
	if (i % 70 != 69)
		fprintf(fp, "\n");
	xfree(buf);
}