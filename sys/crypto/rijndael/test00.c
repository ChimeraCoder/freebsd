
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

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>

#include <crypto/rijndael/rijndael-api-fst.h>

#define LL 32 
int
main(int argc, char **argv)
{
	keyInstance ki;
	cipherInstance ci;
	uint8_t key[16];
	uint8_t in[LL];
	uint8_t out[LL];
	int i, j;

	rijndael_cipherInit(&ci, MODE_CBC, NULL);
	for (i = 0; i < 16; i++)
		key[i] = i;
	rijndael_makeKey(&ki, DIR_DECRYPT, 128, key);
	for (i = 0; i < LL; i++)
		in[i] = i;
	rijndael_blockDecrypt(&ci, &ki, in, LL * 8, out);
	for (i = 0; i < LL; i++)
		printf("%02x", out[i]);
	putchar('\n');
	rijndael_blockDecrypt(&ci, &ki, in, LL * 8, in);
	j = 0;
	for (i = 0; i < LL; i++) {
		printf("%02x", in[i]);
		if (in[i] != out[i])
			j++;
	}
	putchar('\n');
	if (j != 0) {
		fprintf(stderr,
		    "Error: inplace decryption fails in %d places\n", j);
		return (1);
	} else {
		return (0);
	}
}