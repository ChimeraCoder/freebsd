
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

#include <sys/time.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utmpx.h>

static int
b16_pton(const char *in, char *out, size_t len)
{
	size_t i;

	for (i = 0; i < len * 2; i++)
		if (!isxdigit((unsigned char)in[i]))
			return (1);
	for (i = 0; i < len; i++)
		sscanf(&in[i * 2], "%02hhx", &out[i]);
	return (0);
}

static int
rm(char *id[])
{
	struct utmpx utx = { .ut_type = DEAD_PROCESS };
	size_t len;
	int ret = 0;

	(void)gettimeofday(&utx.ut_tv, NULL);
	for (; *id != NULL; id++) {
		len = strlen(*id);
		if (len <= sizeof(utx.ut_id)) {
			/* Identifier as string. */
			strncpy(utx.ut_id, *id, sizeof(utx.ut_id));
		} else if (len != sizeof(utx.ut_id) * 2 ||
		    b16_pton(*id, utx.ut_id, sizeof(utx.ut_id)) != 0) {
			/* Also not hexadecimal. */
			fprintf(stderr, "%s: Invalid identifier format\n", *id);
			ret = 1;
			continue;
		}

		/* Zap the entry. */
		if (pututxline(&utx) == NULL) {
			perror(*id);
			ret = 1;
		}
	}
	return (ret);
}

static int
boot(short type)
{
	struct utmpx utx = { .ut_type = type };

	(void)gettimeofday(&utx.ut_tv, NULL);
	if (pututxline(&utx) == NULL) {
		perror("pututxline");
		return (1);
	}
	return (0);
}

int
main(int argc, char *argv[])
{

	if (argc >= 2 && strcmp(getprogname(), "utxrm") == 0)
		/* For compatibility. */
		return (rm(&argv[1]));
	else if (argc == 2 && strcmp(argv[1], "boot") == 0)
		return (boot(BOOT_TIME));
	else if (argc == 2 && strcmp(argv[1], "shutdown") == 0)
		return (boot(SHUTDOWN_TIME));
	else if (argc >= 3 && strcmp(argv[1], "rm") == 0)
		return (rm(&argv[2]));

	fprintf(stderr,
	    "usage: utx boot\n"
	    "       utx shutdown\n"
	    "       utx rm identifier ...\n"
	    "       utxrm identifier ...\n");
	exit(1);
}