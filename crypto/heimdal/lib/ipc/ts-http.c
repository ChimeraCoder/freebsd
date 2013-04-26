
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
#include <heim-ipc.h>
#include <getarg.h>
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
test_service(void *ctx, const heim_idata *req,
	     const heim_icred cred,
	     heim_ipc_complete complete,
	     heim_sipc_call cctx)
{
    heim_idata rep;
    printf("got request\n");
    rep.length = 3;
    rep.data = strdup("hej");
    (*complete)(cctx, 0, &rep);
}


static void
setup_sockets(void)
{
    struct addrinfo hints, *res, *res0;
    int ret, s;
    heim_sipc u;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    ret = getaddrinfo(NULL, "8080", &hints, &res0);
    if (ret)
	errx(1, "%s", gai_strerror(ret));

    for (res = res0; res ; res = res->ai_next) {
	s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (s < 0) {
	    warn("socket");
	    continue;
	}
	socket_set_reuseaddr(s, 1);
	socket_set_ipv6only(s, 1);

	if (bind(s, res->ai_addr, res->ai_addrlen) < 0) {
	    warn("bind");
	    close(s);
	    continue;
	}
	listen(s, 5);
	ret = heim_sipc_stream_listener(s, HEIM_SIPC_TYPE_HTTP,
					test_service, NULL, &u);
	if (ret)
	    errx(1, "heim_sipc_stream_listener: %d", ret);
    }
    freeaddrinfo(res0);
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

    setup_sockets();

    heim_ipc_main();

    return 0;
}