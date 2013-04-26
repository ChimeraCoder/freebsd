
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
/* $FreeBSD$ */
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>

#define MQNAME	"/mytstqueue1"

int main()
{
	struct mq_attr attr, attr2;
	struct sigevent sigev;
	mqd_t mq;
	int status;

	attr.mq_maxmsg  = 2;
	attr.mq_msgsize = 100;
	mq = mq_open(MQNAME, O_CREAT | O_RDWR | O_EXCL, 0666, &attr);
	if (mq == (mqd_t)-1)
		err(1, "mq_open");
	status = mq_unlink(MQNAME);
	if (status)
		err(1, "mq_unlink");
	status = mq_getattr(mq, &attr2);
	if (status)
		err(1, "mq_getattr");
	if (attr.mq_maxmsg != attr2.mq_maxmsg)
		err(1, "mq_maxmsg changed");
	if (attr.mq_msgsize != attr2.mq_msgsize)
		err(1, "mq_msgsize changed");

	sigev.sigev_notify = SIGEV_SIGNAL;
	sigev.sigev_signo = SIGRTMIN;
	status = mq_notify(mq, &sigev);
	if (status)
		err(1, "mq_notify");
	status = mq_notify(mq, &sigev);
	if (status == 0)
		err(1, "mq_notify 2");
	else if (errno != EBUSY)
		err(1, "mq_notify 3");
	status = mq_notify(mq, NULL);
	if (status)
		err(1, "mq_notify 4");
	status = mq_close(mq);
	if (status)
		err(1, "mq_close");
	return (0);
}