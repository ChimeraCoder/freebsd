
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

#include <sys/types.h>
#include <config/config.h>
#ifdef HAVE_FULL_QUEUE_H
#include <sys/queue.h>
#else /* !HAVE_FULL_QUEUE_H */
#include <compat/queue.h>
#endif /* !HAVE_FULL_QUEUE_H */
#include <sys/uio.h>

#include <bsm/libbsm.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static int send_trigger(int);

#ifdef USE_MACH_IPC
#include <mach/mach.h>
#include <servers/netname.h>
#include <mach/message.h>
#include <mach/port.h>
#include <mach/mach_error.h>
#include <mach/host_special_ports.h>
#include <servers/bootstrap.h>

#include "auditd_control.h"

/* 
 * XXX The following are temporary until these can be added to the kernel
 * audit.h header. 
 */
#ifndef AUDIT_TRIGGER_INITIALIZE
#define	AUDIT_TRIGGER_INITIALIZE	7
#endif
#ifndef AUDIT_TRIGGER_EXPIRE_TRAILS
#define	AUDIT_TRIGGER_EXPIRE_TRAILS	8
#endif

static int
send_trigger(int trigger)
{
	mach_port_t     serverPort;
	kern_return_t	error;

	error = host_get_audit_control_port(mach_host_self(), &serverPort);
	if (error != KERN_SUCCESS) {
		if (geteuid() != 0) {
			errno = EPERM;
			perror("audit requires root privileges"); 
		} else 
			mach_error("Cannot get auditd_control Mach port:",
			    error);
		return (-1);
	}

	error = auditd_control(serverPort, trigger);
	if (error != KERN_SUCCESS) {
		mach_error("Error sending trigger: ", error);
		return (-1);
	}
	
	return (0);
}

#else /* ! USE_MACH_IPC */

static int
send_trigger(int trigger)
{
	int error;

	error = audit_send_trigger(&trigger);
	if (error != 0) {
		if (error == EPERM)
			perror("audit requires root privileges");
		else
			perror("Error sending trigger");
		return (-1);
	}

	return (0);
}
#endif /* ! USE_MACH_IPC */

static void
usage(void)
{

	(void)fprintf(stderr, "Usage: audit -e | -i | -n | -s | -t \n");
	exit(-1);
}

/*
 * Main routine to process command line options.
 */
int
main(int argc, char **argv)
{
	int ch;
	unsigned int trigger = 0;

	if (argc != 2)
		usage();

	while ((ch = getopt(argc, argv, "einst")) != -1) {
		switch(ch) {

		case 'e':
			trigger = AUDIT_TRIGGER_EXPIRE_TRAILS;
			break;

		case 'i':
			trigger = AUDIT_TRIGGER_INITIALIZE;
			break;

		case 'n':
			trigger = AUDIT_TRIGGER_ROTATE_USER;
			break;

		case 's':
			trigger = AUDIT_TRIGGER_READ_FILE;
			break;

		case 't':
			trigger = AUDIT_TRIGGER_CLOSE_AND_DIE;
			break;

		case '?':
		default:
			usage();
			break;
		}
	}
	if (send_trigger(trigger) < 0) 
		exit(-1);

	printf("Trigger sent.\n");
	exit (0);
}