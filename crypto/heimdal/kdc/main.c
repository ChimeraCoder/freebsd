
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

#include "kdc_locl.h"
#ifdef HAVE_UTIL_H
#include <util.h>
#endif

#ifdef HAVE_CAPNG
#include <cap-ng.h>
#endif

sig_atomic_t exit_flag = 0;

#ifdef SUPPORT_DETACH
int detach_from_console = -1;
#endif

static RETSIGTYPE
sigterm(int sig)
{
    exit_flag = sig;
}

/*
 * Allow dropping root bit, since heimdal reopens the database all the
 * time the database needs to be owned by the user you are switched
 * too. A better solution is to split the kdc in to more processes and
 * run the network facing part with very low privilege.
 */

static void
switch_environment(void)
{
#ifdef HAVE_GETEUID
    if ((runas_string || chroot_string) && geteuid() != 0)
	errx(1, "no running as root, can't switch user/chroot");

    if (chroot_string && chroot(chroot_string) != 0)
	errx(1, "chroot(%s)", "chroot_string failed");

    if (runas_string) {
	struct passwd *pw;

	pw = getpwnam(runas_string);
	if (pw == NULL)
	    errx(1, "unknown user %s", runas_string);

	if (initgroups(pw->pw_name, pw->pw_gid) < 0)
	    err(1, "initgroups failed");

#ifndef HAVE_CAPNG
	if (setgid(pw->pw_gid) < 0)
	    err(1, "setgid(%s) failed", runas_string);

	if (setuid(pw->pw_uid) < 0)
	    err(1, "setuid(%s)", runas_string);
#else
	capng_clear (CAPNG_EFFECTIVE | CAPNG_PERMITTED);
	if (capng_updatev (CAPNG_ADD, CAPNG_EFFECTIVE | CAPNG_PERMITTED,
	                   CAP_NET_BIND_SERVICE, CAP_SETPCAP, -1) < 0)
	    err(1, "capng_updateev");

	if (capng_change_id(pw->pw_uid, pw->pw_gid,
	                    CAPNG_CLEAR_BOUNDING) < 0)
	    err(1, "capng_change_id(%s)", runas_string);
#endif
    }
#endif
}


int
main(int argc, char **argv)
{
    krb5_error_code ret;
    krb5_context context;
    krb5_kdc_configuration *config;

    setprogname(argv[0]);

    ret = krb5_init_context(&context);
    if (ret == KRB5_CONFIG_BADFORMAT)
	errx (1, "krb5_init_context failed to parse configuration file");
    else if (ret)
	errx (1, "krb5_init_context failed: %d", ret);

    ret = krb5_kt_register(context, &hdb_kt_ops);
    if (ret)
	errx (1, "krb5_kt_register(HDB) failed: %d", ret);

    config = configure(context, argc, argv);

#ifdef HAVE_SIGACTION
    {
	struct sigaction sa;

	sa.sa_flags = 0;
	sa.sa_handler = sigterm;
	sigemptyset(&sa.sa_mask);

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
#ifdef SIGXCPU
	sigaction(SIGXCPU, &sa, NULL);
#endif

	sa.sa_handler = SIG_IGN;
#ifdef SIGPIPE
	sigaction(SIGPIPE, &sa, NULL);
#endif
    }
#else
    signal(SIGINT, sigterm);
    signal(SIGTERM, sigterm);
#ifdef SIGXCPU
    signal(SIGXCPU, sigterm);
#endif
#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif
#endif
#ifdef SUPPORT_DETACH
    if (detach_from_console)
	daemon(0, 0);
#endif
#ifdef __APPLE__
    bonjour_announce(context, config);
#endif
    pidfile(NULL);

    switch_environment();

    loop(context, config);
    krb5_free_context(context);
    return 0;
}