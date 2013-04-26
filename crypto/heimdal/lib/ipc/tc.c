
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
#include <stdlib.h>
#include <krb5-types.h>
#include <asn1-common.h>
#include <heim-ipc.h>
#include <getarg.h>
#include <err.h>
#include <roken.h>

static int help_flag;
static int version_flag;

static struct getargs args[] = {
    {	"help",		'h',	arg_flag,   &help_flag },
    {	"version",	'v',	arg_flag,   &version_flag }
};

static int num_args = sizeof(args) / sizeof(args[0]);

static void
usage(int ret)
{
    arg_printusage (args, num_args, NULL, "");
    exit (ret);
}

static void
reply(void *ctx, int errorcode, heim_idata *reply, heim_icred cred)
{
    printf("got reply\n");
    heim_ipc_semaphore_signal((heim_isemaphore)ctx); /* tell caller we are done */
}

static void
test_ipc(const char *service)
{
    heim_isemaphore s;
    heim_idata req, rep;
    heim_ipc ipc;
    int ret;

    ret = heim_ipc_init_context(service, &ipc);
    if (ret)
	errx(1, "heim_ipc_init_context: %d", ret);

    req.length = 0;
    req.data = NULL;

    ret = heim_ipc_call(ipc, &req, &rep, NULL);
    if (ret)
	errx(1, "heim_ipc_call: %d", ret);

    s = heim_ipc_semaphore_create(0);
    if (s == NULL)
	errx(1, "heim_ipc_semaphore_create");

    ret = heim_ipc_async(ipc, &req, s, reply);
    if (ret)
	errx(1, "heim_ipc_async: %d", ret);

    heim_ipc_semaphore_wait(s, HEIM_IPC_WAIT_FOREVER); /* wait for reply to complete the work */

    heim_ipc_free_context(ipc);
}


int
main(int argc, char **argv)
{
    int optidx = 0;

    setprogname(argv[0]);

    if (getarg(args, num_args, argc, argv, &optidx))
	usage(1);

    if (help_flag)
	usage(0);

    if (version_flag) {
	print_version(NULL);
	exit(0);
    }

#ifdef __APPLE__
    test_ipc("MACH:org.h5l.test-ipc");
#endif
    test_ipc("ANY:org.h5l.test-ipc");
    test_ipc("UNIX:org.h5l.test-ipc");

    return 0;
}