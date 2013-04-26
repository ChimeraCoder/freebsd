
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

#if defined(HAVE_LIBIAF)  &&  !defined(HAVE_SECUREWARE)
#include <sys/types.h>
#ifdef HAVE_CRYPT_H
# include <crypt.h>
#endif
#include <pwd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "xmalloc.h"
#include "packet.h"
#include "buffer.h"
#include "key.h"
#include "auth-options.h"
#include "log.h"
#include "servconf.h"
#include "hostfile.h"
#include "auth.h"
#include "ssh.h"

int nischeck(char *);

int
sys_auth_passwd(Authctxt *authctxt, const char *password)
{
	struct passwd *pw = authctxt->pw;
	char *salt;
	int result;

	/* Just use the supplied fake password if authctxt is invalid */
	char *pw_password = authctxt->valid ? shadow_pw(pw) : pw->pw_passwd;

	/* Check for users with no password. */
	if (strcmp(pw_password, "") == 0 && strcmp(password, "") == 0)
		return (1);

	/* Encrypt the candidate password using the proper salt. */
	salt = (pw_password[0] && pw_password[1]) ? pw_password : "xx";

	/*
	 * Authentication is accepted if the encrypted passwords
	 * are identical.
	 */
#ifdef UNIXWARE_LONG_PASSWORDS
	if (!nischeck(pw->pw_name)) {
		result = ((strcmp(bigcrypt(password, salt), pw_password) == 0)
		||  (strcmp(osr5bigcrypt(password, salt), pw_password) == 0));
	}
	else
#endif /* UNIXWARE_LONG_PASSWORDS */
		result = (strcmp(xcrypt(password, salt), pw_password) == 0);

#ifdef USE_LIBIAF
	if (authctxt->valid)
		free(pw_password);
#endif
	return(result);
}

#ifdef UNIXWARE_LONG_PASSWORDS
int
nischeck(char *namep)
{
	char password_file[] = "/etc/passwd";
	FILE *fd;
	struct passwd *ent = NULL;

	if ((fd = fopen (password_file, "r")) == NULL) {
		/*
		 * If the passwd file has dissapeared we are in a bad state.
		 * However, returning 0 will send us back through the
		 * authentication scheme that has checked the ia database for
		 * passwords earlier.
		 */
		return(0);
	}

	/*
	 * fgetpwent() only reads from password file, so we know for certain
	 * that the user is local.
	 */
	while (ent = fgetpwent(fd)) {
		if (strcmp (ent->pw_name, namep) == 0) {
			/* Local user */
			fclose (fd);
			return(0);
		}
	}

	fclose (fd);
	return (1);
}

#endif /* UNIXWARE_LONG_PASSWORDS */

/*
	NOTE: ia_get_logpwd() allocates memory for arg 2
	functions that call shadow_pw() will need to free
 */

#ifdef USE_LIBIAF
char *
get_iaf_password(struct passwd *pw)
{
	char *pw_password = NULL;

	uinfo_t uinfo;
	if (!ia_openinfo(pw->pw_name,&uinfo)) {
		ia_get_logpwd(uinfo, &pw_password);
		if (pw_password == NULL)
			fatal("ia_get_logpwd: Unable to get the shadow passwd");
		ia_closeinfo(uinfo);
	 	return pw_password;
	}
	else
		fatal("ia_openinfo: Unable to open the shadow passwd file");
}
#endif /* USE_LIBIAF */
#endif /* HAVE_LIBIAF and not HAVE_SECUREWARE */