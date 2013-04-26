
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
#include <unistd.h>
#include <pwd.h>

# if defined(HAVE_CRYPT_H) && !defined(HAVE_SECUREWARE)
#  include <crypt.h>
# endif

# ifdef __hpux
#  include <hpsecurity.h>
#  include <prot.h>
# endif

# ifdef HAVE_SECUREWARE
#  include <sys/security.h>
#  include <sys/audit.h>
#  include <prot.h>
# endif 

# if defined(HAVE_SHADOW_H) && !defined(DISABLE_SHADOW)
#  include <shadow.h>
# endif

# if defined(HAVE_GETPWANAM) && !defined(DISABLE_SHADOW)
#  include <sys/label.h>
#  include <sys/audit.h>
#  include <pwdadj.h>
# endif

# if defined(HAVE_MD5_PASSWORDS) && !defined(HAVE_MD5_CRYPT)
#  include "md5crypt.h"
# endif 

char *
xcrypt(const char *password, const char *salt)
{
	char *crypted;

# ifdef HAVE_MD5_PASSWORDS
        if (is_md5_salt(salt))
                crypted = md5_crypt(password, salt);
        else
                crypted = crypt(password, salt);
# elif defined(__hpux) && !defined(HAVE_SECUREWARE)
	if (iscomsec())
                crypted = bigcrypt(password, salt);
        else
                crypted = crypt(password, salt);
# elif defined(HAVE_SECUREWARE)
        crypted = bigcrypt(password, salt);
# else
        crypted = crypt(password, salt);
# endif 

	return crypted;
}

/*
 * Handle shadowed password systems in a cleaner way for portable
 * version.
 */

char *
shadow_pw(struct passwd *pw)
{
	char *pw_password = pw->pw_passwd;

# if defined(HAVE_SHADOW_H) && !defined(DISABLE_SHADOW)
	struct spwd *spw = getspnam(pw->pw_name);

	if (spw != NULL)
		pw_password = spw->sp_pwdp;
# endif

#ifdef USE_LIBIAF
	return(get_iaf_password(pw));
#endif

# if defined(HAVE_GETPWANAM) && !defined(DISABLE_SHADOW)
	struct passwd_adjunct *spw;
	if (issecure() && (spw = getpwanam(pw->pw_name)) != NULL)
		pw_password = spw->pwa_passwd;
# elif defined(HAVE_SECUREWARE)
	struct pr_passwd *spw = getprpwnam(pw->pw_name);

	if (spw != NULL)
		pw_password = spw->ufld.fd_encrypt;
# endif

	return pw_password;
}