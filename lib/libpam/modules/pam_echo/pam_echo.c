
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/openpam.h>

static int
_pam_echo(pam_handle_t *pamh, int flags,
    int argc, const char *argv[])
{
	char msg[PAM_MAX_MSG_SIZE];
	const void *str;
	const char *p, *q;
	int err, i, item;
	size_t len;

	if (flags & PAM_SILENT)
		return (PAM_SUCCESS);
	for (i = 0, len = 0; i < argc && len < sizeof(msg) - 1; ++i) {
		if (i > 0)
			msg[len++] = ' ';
		for (p = argv[i]; *p != '\0' && len < sizeof(msg) - 1; ++p) {
			if (*p != '%' || p[1] == '\0') {
				msg[len++] = *p;
				continue;
			}
			switch (*++p) {
			case 'H':
				item = PAM_RHOST;
				break;
			case 'h':
				/* not implemented */
				item = -1;
				break;
			case 's':
				item = PAM_SERVICE;
				break;
			case 't':
				item = PAM_TTY;
				break;
			case 'U':
				item = PAM_RUSER;
				break;
			case 'u':
				item = PAM_USER;
				break;
			default:
				item = -1;
				msg[len++] = *p;
				break;
			}
			if (item == -1)
				continue;
			err = pam_get_item(pamh, item, &str);
			if (err != PAM_SUCCESS)
				return (err);
			if (str == NULL)
				str = "(null)";
			for (q = str; *q != '\0' && len < sizeof(msg) - 1; ++q)
				msg[len++] = *q;
		}
	}
	msg[len] = '\0';
	return (pam_info(pamh, "%s", msg));
}

PAM_EXTERN int
pam_sm_authenticate(pam_handle_t *pamh, int flags,
    int argc, const char *argv[])
{

	return (_pam_echo(pamh, flags, argc, argv));
}

PAM_EXTERN int
pam_sm_setcred(pam_handle_t *pamh __unused, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{

	return (PAM_SUCCESS);
}

PAM_EXTERN int
pam_sm_acct_mgmt(pam_handle_t *pamh, int flags,
    int argc, const char *argv[])
{

	return (_pam_echo(pamh, flags, argc, argv));
}

PAM_EXTERN int
pam_sm_open_session(pam_handle_t *pamh, int flags,
    int argc, const char *argv[])
{

	return (_pam_echo(pamh, flags, argc, argv));
}

PAM_EXTERN int
pam_sm_close_session(pam_handle_t *pamh, int flags,
    int argc, const char *argv[])
{

	return (_pam_echo(pamh, flags, argc, argv));
}

PAM_EXTERN int
pam_sm_chauthtok(pam_handle_t *pamh, int flags,
    int argc, const char *argv[])
{

	if (flags & PAM_PRELIM_CHECK)
		return (PAM_SUCCESS);
	return (_pam_echo(pamh, flags, argc, argv));
}

PAM_MODULE_ENTRY("pam_echo");