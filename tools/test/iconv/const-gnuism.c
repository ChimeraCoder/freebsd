
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

#include <sys/types.h>

#include <err.h>
#include <iconv.h>
#include <stdlib.h>
#include <string.h>

int
main(void)
{
	iconv_t cd;
	size_t inbytes, outbytes;
	char *str1 = "FOOBAR";
	const char *str2 = "FOOBAR";
	char ** in1;
	const char ** in2 = &str2;
	char *out1, *out2;

	inbytes = outbytes = strlen("FOOBAR");

	if ((cd = iconv_open("UTF-8", "ASCII")) == (iconv_t)-1)
		err(1, NULL);

	if ((out2 = malloc(inbytes)) == NULL)
		err(1, NULL);

	if (iconv(cd, in2, &inbytes, &out2, &outbytes) == -1)
		err(1, NULL);

	in1 = &str1;
	inbytes = outbytes = strlen("FOOBAR");

	if ((out1 = malloc(inbytes)) == NULL)
		err(1, NULL);

	if (iconv(cd, in1, &inbytes, &out1, &outbytes) == -1)
		err(1, NULL);

	return (EXIT_SUCCESS);

}