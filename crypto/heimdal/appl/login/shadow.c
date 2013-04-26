
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

#include "login_locl.h"

RCSID("$Id$");

#ifdef HAVE_SHADOW_H

#ifndef _PATH_CHPASS
#define _PATH_CHPASS "/usr/bin/passwd"
#endif

static int
change_passwd(const struct passwd *who)
{
    int status;
    pid_t pid;

    switch (pid = fork()) {
    case -1:
        printf("fork /bin/passwd");
        exit(1);
    case 0:
        execlp(_PATH_CHPASS, "passwd", who->pw_name, (char *) 0);
        exit(1);
    default:
        waitpid(pid, &status, 0);
        return (status);
    }
}

void
check_shadow(const struct passwd *pw, const struct spwd *sp)
{
  long today;

  today = time(0)/(24L * 60 * 60);

  if (sp == NULL)
      return;

  if (sp->sp_expire > 0) {
        if (today >= sp->sp_expire) {
            printf("Your account has expired.\n");
            sleep(1);
            exit(0);
        } else if (sp->sp_expire - today < 14) {
            printf("Your account will expire in %d days.\n",
                   (int)(sp->sp_expire - today));
        }
  }

  if (sp->sp_max > 0) {
        if (today >= (sp->sp_lstchg + sp->sp_max)) {
            printf("Your password has expired. Choose a new one.\n");
            change_passwd(pw);
        } else if (sp->sp_warn > 0
            && (today > (sp->sp_lstchg + sp->sp_max - sp->sp_warn))) {
            printf("Your password will expire in %d days.\n",
                   (int)(sp->sp_lstchg + sp->sp_max - today));
        }
  }
}
#endif /* HAVE_SHADOW_H */