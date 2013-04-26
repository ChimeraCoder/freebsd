
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

#include "kcm_locl.h"

RCSID("$Id$");

sig_atomic_t exit_flag = 0;

krb5_context kcm_context = NULL;

const char *service_name = "org.h5l.kcm";

static RETSIGTYPE
sigterm(int sig)
{
    exit_flag = 1;
}

static RETSIGTYPE
sigusr1(int sig)
{
    kcm_debug_ccache(kcm_context);
}

static RETSIGTYPE
sigusr2(int sig)
{
    kcm_debug_events(kcm_context);
}

int
main(int argc, char **argv)
{
    krb5_error_code ret;
    setprogname(argv[0]);

    ret = krb5_init_context(&kcm_context);
    if (ret) {
	errx (1, "krb5_init_context failed: %d", ret);
	return ret;
    }

    kcm_configure(argc, argv);

#ifdef HAVE_SIGACTION
    {
	struct sigaction sa;

	sa.sa_flags = 0;
	sa.sa_handler = sigterm;
	sigemptyset(&sa.sa_mask);

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	sa.sa_handler = sigusr1;
	sigaction(SIGUSR1, &sa, NULL);

	sa.sa_handler = sigusr2;
	sigaction(SIGUSR2, &sa, NULL);

	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, NULL);
    }
#else
    signal(SIGINT, sigterm);
    signal(SIGTERM, sigterm);
    signal(SIGUSR1, sigusr1);
    signal(SIGUSR2, sigusr2);
    signal(SIGPIPE, SIG_IGN);
#endif
#ifdef SUPPORT_DETACH
    if (detach_from_console)
	daemon(0, 0);
#endif
    pidfile(NULL);

    if (launchd_flag) {
	heim_sipc mach;
	heim_sipc_launchd_mach_init(service_name, kcm_service, NULL, &mach);
    } else {
	heim_sipc un;
	heim_sipc_service_unix(service_name, kcm_service, NULL, &un);
    }

    heim_ipc_main();

    krb5_free_context(kcm_context);
    return 0;
}