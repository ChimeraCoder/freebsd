
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>

#define MSG_SIZE 1024
#define PORT 6969

int
main(int argc, char **argv)
{

	int sock;
	int maxhold;
	size_t size = sizeof(maxhold);
	struct sockaddr_in dest;
	char message[MSG_SIZE];
	struct arpstat arpstat;
	size_t len = sizeof(arpstat);
	unsigned long dropped = 0;

	memset(&message, 1, sizeof(message));

	if (sysctlbyname("net.link.ether.inet.maxhold", &maxhold, &size,
			 NULL, 0) < 0) {
		perror("not ok 1 - sysctlbyname failed");
		exit(1);
	}
	    
#ifdef DEBUG
	printf("maxhold is %d\n", maxhold);
#endif /* DEBUG */

	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("not ok 1 - could not open socket");
		exit(1);
	}

	bzero(&dest, sizeof(dest));
	if (inet_pton(AF_INET, argv[1], &dest.sin_addr.s_addr) != 1) {
		perror("not ok 1 - could not parse address");
		exit(1);
	}
	dest.sin_len = sizeof(dest);
	dest.sin_family = AF_INET;
	dest.sin_port = htons(PORT);

	if (sysctlbyname("net.link.ether.arp.stats", &arpstat, &len,
			 NULL, 0) < 0) {
		perror("not ok 1 - could not get initial arp stats");
		exit(1);
	}

	dropped = arpstat.dropped;
#ifdef DEBUG
	printf("dropped before %ld\n", dropped);
#endif /* DEBUG */

	/* 
	 * Load up the queue in the ARP entry to the maximum.
	 * We should not drop any packets at this point. 
	 */

	while (maxhold > 0) {
		if (sendto(sock, message, sizeof(message), 0,
			   (struct sockaddr *)&dest, sizeof(dest)) < 0) {
			perror("not ok 1 - could not send packet");
			exit(1);
		}
		maxhold--;
	}

	if (sysctlbyname("net.link.ether.arp.stats", &arpstat, &len,
			 NULL, 0) < 0) {
		perror("not ok 1 - could not get new arp stats");
		exit(1);
	}

#ifdef DEBUG
	printf("dropped after %ld\n", arpstat.dropped);
#endif /* DEBUG */

	if (arpstat.dropped != dropped) {
		printf("not ok 1 - Failed, drops changed:"
		       "before %ld after %ld\n", dropped, arpstat.dropped);
		exit(1);
	}
	
	dropped = arpstat.dropped;

	/* Now add one extra and make sure it is dropped. */
	if (sendto(sock, message, sizeof(message), 0,
		   (struct sockaddr *)&dest, sizeof(dest)) < 0) {
		perror("not ok 1 - could not send packet");
		exit(1);
	}

	if (sysctlbyname("net.link.ether.arp.stats", &arpstat, &len,
			 NULL, 0) < 0) {
		perror("not ok 1 - could not get new arp stats");
		exit(1);
	}

	if (arpstat.dropped != (dropped + 1)) {
		printf("not ok 1 - Failed to drop one packet: before"
		       " %ld after %ld\n", dropped, arpstat.dropped);
		exit(1);
	}

	printf("ok\n");
	return (0);
}