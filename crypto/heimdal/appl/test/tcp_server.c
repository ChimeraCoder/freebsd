
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

#include "test_locl.h"
RCSID("$Id$");

krb5_context context;

static int
proto (int sock, const char *service)
{
    krb5_auth_context auth_context;
    krb5_error_code status;
    krb5_principal server;
    krb5_ticket *ticket;
    char *name;
    char hostname[MAXHOSTNAMELEN];
    krb5_data packet;
    krb5_data data;
    uint32_t len, net_len;
    ssize_t n;

    status = krb5_auth_con_init (context, &auth_context);
    if (status)
	krb5_err (context, 1, status, "krb5_auth_con_init");

    status = krb5_auth_con_setaddrs_from_fd (context,
					     auth_context,
					     &sock);

    if (status)
	krb5_err (context, 1, status, "krb5_auth_con_setaddrs_from_fd");

    if(gethostname (hostname, sizeof(hostname)) < 0)
	krb5_err (context, 1, errno, "gethostname");

    status = krb5_sname_to_principal (context,
				      hostname,
				      service,
				      KRB5_NT_SRV_HST,
				      &server);
    if (status)
	krb5_err (context, 1, status, "krb5_sname_to_principal");

    status = krb5_recvauth (context,
			    &auth_context,
			    &sock,
			    VERSION,
			    server,
			    0,
			    keytab,
			    &ticket);
    if (status)
	krb5_err (context, 1, status, "krb5_recvauth");

    status = krb5_unparse_name (context,
				ticket->client,
				&name);
    if (status)
	krb5_err (context, 1, status, "krb5_unparse_name");

    fprintf (stderr, "User is `%s'\n", name);
    free (name);

    krb5_data_zero (&data);
    krb5_data_zero (&packet);

    n = krb5_net_read (context, &sock, &net_len, 4);
    if (n == 0)
	krb5_errx (context, 1, "EOF in krb5_net_read");
    if (n < 0)
	krb5_err (context, 1, errno, "krb5_net_read");

    len = ntohl(net_len);

    krb5_data_alloc (&packet, len);

    n = krb5_net_read (context, &sock, packet.data, len);
    if (n == 0)
	krb5_errx (context, 1, "EOF in krb5_net_read");
    if (n < 0)
	krb5_err (context, 1, errno, "krb5_net_read");

    status = krb5_rd_safe (context,
			   auth_context,
			   &packet,
			   &data,
			   NULL);
    if (status)
	krb5_err (context, 1, status, "krb5_rd_safe");

    fprintf (stderr, "safe packet: %.*s\n", (int)data.length,
	    (char *)data.data);

    n = krb5_net_read (context, &sock, &net_len, 4);
    if (n == 0)
	krb5_errx (context, 1, "EOF in krb5_net_read");
    if (n < 0)
	krb5_err (context, 1, errno, "krb5_net_read");

    len = ntohl(net_len);

    krb5_data_alloc (&packet, len);

    n = krb5_net_read (context, &sock, packet.data, len);
    if (n == 0)
	krb5_errx (context, 1, "EOF in krb5_net_read");
    if (n < 0)
	krb5_err (context, 1, errno, "krb5_net_read");

    status = krb5_rd_priv (context,
			   auth_context,
			   &packet,
			   &data,
			   NULL);
    if (status)
	krb5_err (context, 1, status, "krb5_rd_priv");

    fprintf (stderr, "priv packet: %.*s\n", (int)data.length,
	    (char *)data.data);

    return 0;
}

static int
doit (int port, const char *service)
{
    mini_inetd (port, NULL);

    return proto (STDIN_FILENO, service);
}

int
main(int argc, char **argv)
{
    int port = server_setup(&context, argc, argv);
    return doit (port, service);
}