
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

#include <sys/cdefs.h>

__FBSDID("$FreeBSD$");

#ifndef lint
static const char sccsid[] = "@(#)io.c	8.1 (Berkeley) 6/6/93";
#endif

/*
 * This file contains the I/O handling and the exchange of
 * edit characters. This connection itself is established in
 * ctl.c
 */

#include <sys/filio.h>

#include <errno.h>
#include <signal.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "talk.h"
#include "talk_ctl.h"

#define A_LONG_TIME 10000000

volatile sig_atomic_t gotwinch = 0;

/*
 * The routine to do the actual talking
 */
void
talk(void)
{
	struct hostent *hp, *hp2;
	int nb;
	fd_set read_set, read_template;
	char buf[BUFSIZ], **addr, *his_machine_name;
	struct timeval wait;

	his_machine_name = NULL;
	hp = gethostbyaddr((const char *)&his_machine_addr.s_addr,
	    sizeof(his_machine_addr.s_addr), AF_INET);
	if (hp != NULL) {
		hp2 = gethostbyname(hp->h_name);
		if (hp2 != NULL && hp2->h_addrtype == AF_INET &&
		    hp2->h_length == sizeof(his_machine_addr))
			for (addr = hp2->h_addr_list; *addr != NULL; addr++)
				if (memcmp(*addr, &his_machine_addr,
				    sizeof(his_machine_addr)) == 0) {
					his_machine_name = strdup(hp->h_name);
					break;
				}
	}
	if (his_machine_name == NULL)
		his_machine_name = strdup(inet_ntoa(his_machine_addr));
	snprintf(buf, sizeof(buf), "Connection established with %s@%s.",
	    msg.r_name, his_machine_name);
	free(his_machine_name);
	message(buf);
	write(STDOUT_FILENO, "\007\007\007", 3);
	
	current_line = 0;

	/*
	 * Wait on both the other process (sockt_mask) and
	 * standard input ( STDIN_MASK )
	 */
	FD_ZERO(&read_template);
	FD_SET(sockt, &read_template);
	FD_SET(fileno(stdin), &read_template);
	for (;;) {
		read_set = read_template;
		wait.tv_sec = A_LONG_TIME;
		wait.tv_usec = 0;
		nb = select(32, &read_set, 0, 0, &wait);
		if (gotwinch) {
			resize_display();
			gotwinch = 0;
		}
		if (nb <= 0) {
			if (errno == EINTR) {
				read_set = read_template;
				continue;
			}
			/* panic, we don't know what happened */
			p_error("Unexpected error from select");
			quit();
		}
		if (FD_ISSET(sockt, &read_set)) {
			/* There is data on sockt */
			nb = read(sockt, buf, sizeof buf);
			if (nb <= 0) {
				message("Connection closed. Exiting");
				quit();
			}
			display(&his_win, buf, nb);
		}
		if (FD_ISSET(fileno(stdin), &read_set)) {
			/*
			 * We can't make the tty non_blocking, because
			 * curses's output routines would screw up
			 */
			int i;
			ioctl(0, FIONREAD, (void *) &nb);
			if (nb > (ssize_t)(sizeof buf))
				nb = sizeof buf;
			nb = read(STDIN_FILENO, buf, nb);
			display(&my_win, buf, nb);
			/* might lose data here because sockt is non-blocking */
			for (i = 0; i < nb; ++i)
				if (buf[i] == '\r')
					buf[i] = '\n';
			write(sockt, buf, nb);
		}
	}
}

/*
 * p_error prints the system error message on the standard location
 * on the screen and then exits. (i.e. a curses version of perror)
 */
void
p_error(const char *string)
{
	wmove(my_win.x_win, current_line, 0);
	wprintw(my_win.x_win, "[%s : %s (%d)]\n",
	    string, strerror(errno), errno);
	wrefresh(my_win.x_win);
	move(LINES-1, 0);
	refresh();
	quit();
}

/*
 * Display string in the standard location
 */
void
message(const char *string)
{
	wmove(my_win.x_win, current_line, 0);
	wprintw(my_win.x_win, "[%s]\n", string);
	if (current_line < my_win.x_nlines - 1)
		current_line++;
	wrefresh(my_win.x_win);
}