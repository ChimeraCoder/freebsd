
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

#include <sys/param.h>
#include <sys/jail.h>
#include <sys/socket.h>
#include <sys/sysctl.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <err.h>
#include <errno.h>
#include <jail.h>
#include <limits.h>
#include <login_cap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>

static void	usage(void);

#define GET_USER_INFO do {						\
	pwd = getpwnam(username);					\
	if (pwd == NULL) {						\
		if (errno)						\
			err(1, "getpwnam: %s", username);		\
		else							\
			errx(1, "%s: no such user", username);		\
	}								\
	lcap = login_getpwclass(pwd);					\
	if (lcap == NULL)						\
		err(1, "getpwclass: %s", username);			\
	ngroups = ngroups_max;						\
	if (getgrouplist(username, pwd->pw_gid, groups, &ngroups) != 0)	\
		err(1, "getgrouplist: %s", username);			\
} while (0)

int
main(int argc, char *argv[])
{
	int jid;
	login_cap_t *lcap = NULL;
	struct passwd *pwd = NULL;
	gid_t *groups = NULL;
	int ch, ngroups, uflag, Uflag;
	long ngroups_max;
	char *username;

	ch = uflag = Uflag = 0;
	username = NULL;
	ngroups_max = sysconf(_SC_NGROUPS_MAX) + 1;
	if ((groups = malloc(sizeof(gid_t) * ngroups_max)) == NULL)
		err(1, "malloc");

	while ((ch = getopt(argc, argv, "nu:U:")) != -1) {
		switch (ch) {
		case 'n':
			/* Specified name, now unused */
			break;
		case 'u':
			username = optarg;
			uflag = 1;
			break;
		case 'U':
			username = optarg;
			Uflag = 1;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (argc < 2)
		usage();
	if (uflag && Uflag)
		usage();
	if (uflag)
		GET_USER_INFO;
	jid = jail_getid(argv[0]);
	if (jid < 0)
		errx(1, "%s", jail_errmsg);
	if (jail_attach(jid) == -1)
		err(1, "jail_attach(%d)", jid);
	if (chdir("/") == -1)
		err(1, "chdir(): /");
	if (username != NULL) {
		if (Uflag)
			GET_USER_INFO;
		if (setgroups(ngroups, groups) != 0)
			err(1, "setgroups");
		if (setgid(pwd->pw_gid) != 0)
			err(1, "setgid");
		if (setusercontext(lcap, pwd, pwd->pw_uid,
		    LOGIN_SETALL & ~LOGIN_SETGROUP & ~LOGIN_SETLOGIN) != 0)
			err(1, "setusercontext");
		login_close(lcap);
	}
	if (execvp(argv[1], argv + 1) == -1)
		err(1, "execvp(): %s", argv[1]);
	exit(0);
}

static void
usage(void)
{

	fprintf(stderr, "%s\n",
		"usage: jexec [-u username | -U username] jail command ...");
	exit(1); 
}