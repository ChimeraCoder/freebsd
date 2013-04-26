
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

#include "namespace.h"
#include <sys/endian.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <utmpx.h>
#include "utxdb.h"
#include "un-namespace.h"

#ifdef __NO_TLS
static FILE *uf = NULL;
static int udb;
#else
static _Thread_local FILE *uf = NULL;
static _Thread_local int udb;
#endif

int
setutxdb(int db, const char *file)
{
	struct stat sb;

	switch (db) {
	case UTXDB_ACTIVE:
		if (file == NULL)
			file = _PATH_UTX_ACTIVE;
		break;
	case UTXDB_LASTLOGIN:
		if (file == NULL)
			file = _PATH_UTX_LASTLOGIN;
		break;
	case UTXDB_LOG:
		if (file == NULL)
			file = _PATH_UTX_LOG;
		break;
	default:
		errno = EINVAL;
		return (-1);
	}

	if (uf != NULL)
		fclose(uf);
	uf = fopen(file, "re");
	if (uf == NULL)
		return (-1);

	if (db != UTXDB_LOG) {
		/* Safety check: never use broken files. */
		if (_fstat(fileno(uf), &sb) != -1 &&
		    sb.st_size % sizeof(struct futx) != 0) {
			fclose(uf);
			uf = NULL;
			errno = EFTYPE;
			return (-1);
		}
		/* Prevent reading of partial records. */
		(void)setvbuf(uf, NULL, _IOFBF,
		    rounddown(BUFSIZ, sizeof(struct futx)));
	}

	udb = db;
	return (0);
}

void
setutxent(void)
{

	setutxdb(UTXDB_ACTIVE, NULL);
}

void
endutxent(void)
{

	if (uf != NULL) {
		fclose(uf);
		uf = NULL;
	}
}

static int
getfutxent(struct futx *fu)
{

	if (uf == NULL)
		setutxent();
	if (uf == NULL)
		return (-1);

	if (udb == UTXDB_LOG) {
		uint16_t len;

		if (fread(&len, sizeof(len), 1, uf) != 1)
			return (-1);
		len = be16toh(len);
		if (len > sizeof *fu) {
			/* Forward compatibility. */
			if (fread(fu, sizeof(*fu), 1, uf) != 1)
				return (-1);
			fseek(uf, len - sizeof(*fu), SEEK_CUR);
		} else {
			/* Partial record. */
			memset(fu, 0, sizeof(*fu));
			if (fread(fu, len, 1, uf) != 1)
				return (-1);
		}
	} else {
		if (fread(fu, sizeof(*fu), 1, uf) != 1)
			return (-1);
	}
	return (0);
}

struct utmpx *
getutxent(void)
{
	struct futx fu;

	if (getfutxent(&fu) != 0)
		return (NULL);
	return (futx_to_utx(&fu));
}

struct utmpx *
getutxid(const struct utmpx *id)
{
	struct futx fu;

	for (;;) {
		if (getfutxent(&fu) != 0)
			return (NULL);

		switch (fu.fu_type) {
		case USER_PROCESS:
		case INIT_PROCESS:
		case LOGIN_PROCESS:
		case DEAD_PROCESS:
			switch (id->ut_type) {
			case USER_PROCESS:
			case INIT_PROCESS:
			case LOGIN_PROCESS:
			case DEAD_PROCESS:
				if (memcmp(fu.fu_id, id->ut_id,
				    MIN(sizeof(fu.fu_id), sizeof(id->ut_id))) ==
				    0)
					goto found;
			}
			break;
		default:
			if (fu.fu_type == id->ut_type)
				goto found;
			break;
		}
	}

found:
	return (futx_to_utx(&fu));
}

struct utmpx *
getutxline(const struct utmpx *line)
{
	struct futx fu;

	for (;;) {
		if (getfutxent(&fu) != 0)
			return (NULL);

		switch (fu.fu_type) {
		case USER_PROCESS:
		case LOGIN_PROCESS:
			if (strncmp(fu.fu_line, line->ut_line,
			    MIN(sizeof(fu.fu_line), sizeof(line->ut_line))) ==
			    0)
				goto found;
			break;
		}
	}

found:
	return (futx_to_utx(&fu));
}

struct utmpx *
getutxuser(const char *user)
{
	struct futx fu;

	for (;;) {
		if (getfutxent(&fu) != 0)
			return (NULL);

		switch (fu.fu_type) {
		case USER_PROCESS:
			if (strncmp(fu.fu_user, user, sizeof(fu.fu_user)) == 0)
				goto found;
			break;
		}
	}

found:
	return (futx_to_utx(&fu));
}