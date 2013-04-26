
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
/*
 * $FreeBSD$
 */void quit(const char *text)
{
	err(errno, text);
}

char *sched_text(int scheduler)
{
	switch(scheduler)
	{
		case SCHED_FIFO:
		return "SCHED_FIFO";

		case SCHED_RR:
		return "SCHED_RR";

		case SCHED_OTHER:
		return "SCHED_OTHER";

		default:
		return "Illegal scheduler value";
	}
}

int sched_is(int line, struct sched_param *p, int shouldbe)
{
	int scheduler;
	struct sched_param param;

	/* What scheduler are we running now?
	 */
	errno = 0;
	scheduler = sched_getscheduler(0);
	if (sched_getparam(0, &param))
		quit("sched_getparam");

	if (p)
		*p = param;

	if (shouldbe != -1 && scheduler != shouldbe)
	{
		fprintf(stderr,
		"At line %d the scheduler should be %s yet it is %s.\n",
		line, sched_text(shouldbe), sched_text(scheduler));

		exit(-1);
	}

	return scheduler;
}