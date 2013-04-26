
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

#include <bluetooth.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int  hostmode  (char const *arg, int brief);
static int  protomode (char const *arg, int brief);
static void usage     (void);

int
main(int argc, char **argv)
{
	int	opt, brief = 0, proto = 0;

	while ((opt = getopt(argc, argv, "bhp")) != -1) {
		switch (opt) {
		case 'b':
			brief = 1;
			break;

		case 'p':
			proto = 1;
			break;

		case 'h':
		default:
			usage();
			/* NOT REACHED */
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 1)
		usage();

	exit(proto? protomode(*argv, brief) : hostmode(*argv, brief));
}

static int
hostmode(char const *arg, int brief)
{
	struct hostent	*he = NULL;
	bdaddr_t	 ba;
	char		 bastr[32];
	int		 reverse;

	if (bt_aton(arg, &ba) == 1) {
		reverse = 1;
		he = bt_gethostbyaddr((char const *) &ba, sizeof(ba), 
					AF_BLUETOOTH);
	} else {
		reverse = 0;
		he = bt_gethostbyname(arg);
	}

	if (he == NULL) {
		herror(reverse? bt_ntoa(&ba, bastr) : arg);
		return (1);
	}

	if (brief)
		printf("%s", reverse? he->h_name :
				bt_ntoa((bdaddr_t *)(he->h_addr), bastr));
	else
		printf("Host %s has %s %s\n", 
			reverse? bt_ntoa(&ba, bastr) : arg,
			reverse? "name" : "address",
			reverse? he->h_name :
				bt_ntoa((bdaddr_t *)(he->h_addr), bastr));

	return (0);
}

static int
protomode(char const *arg, int brief)
{
	struct protoent	*pe = NULL;
	int		 proto;

	if ((proto = atoi(arg)) != 0)
		pe = bt_getprotobynumber(proto);
	else
		pe = bt_getprotobyname(arg);

	if (pe == NULL) {
		fprintf(stderr, "%s: Unknown Protocol/Service Multiplexor\n", arg);
		return (1);
	}

	if (brief) {
		if (proto)
			printf("%s", pe->p_name);
		else
			printf("%d", pe->p_proto);
	} else {
		printf("Protocol/Service Multiplexor %s has number %d\n",
			pe->p_name, pe->p_proto);
	}

	return (0);
}

static void
usage(void)
{
	fprintf(stdout, "Usage: bthost [-b -h -p] host_or_protocol\n");
	exit(255);
}