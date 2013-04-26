
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
#include <namespace.h>
#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include <assert.h>
#include <sys/time.h>
#include "printf.h"

int
__printf_arginfo_time(const struct printf_info *pi, size_t n, int *argt)
{

	assert(n >= 1);
	argt[0] = PA_POINTER;
	return (1);
}
#define MINUTE 60
#define HOUR	(60 * MINUTE)
#define DAY	(24 * HOUR)
#define YEAR	(365 * DAY)

int
__printf_render_time(struct __printf_io *io, const struct printf_info *pi, const void *const *arg)
{
	char buf[100];
	char *p;
	struct timeval *tv;
	struct timespec *ts;
	time_t *tp;
	intmax_t t, tx;
	int i, prec, nsec, ret;

	if (pi->is_long) {
		tv = *((struct timeval **)arg[0]);
		t = tv->tv_sec;
		nsec = tv->tv_usec * 1000;
		prec = 6;
	} else if (pi->is_long_double) {
		ts = *((struct timespec **)arg[0]);
		t = ts->tv_sec;
		nsec = ts->tv_nsec;
		prec = 9;
	} else {
		tp = *((time_t **)arg[0]);
		t = *tp;
		nsec = 0;
		prec = 0;
	}
	if (pi->is_long || pi->is_long_double) {
		if (pi->prec >= 0) 
			prec = pi->prec;
		if (prec == 0)
			nsec = 0;
	}

	p = buf;
	if (pi->alt) {
		tx = t;
		if (t >= YEAR) {
			p += sprintf(p, "%jdy", t / YEAR);
			t %= YEAR;
		}
		if (tx >= DAY && (t != 0 || prec != 0)) {
			p += sprintf(p, "%jdd", t / DAY);
			t %= DAY;
		}
		if (tx >= HOUR && (t != 0 || prec != 0)) {
			p += sprintf(p, "%jdh", t / HOUR);
			t %= HOUR;
		}
		if (tx >= MINUTE && (t != 0 || prec != 0)) {
			p += sprintf(p, "%jdm", t / MINUTE);
			t %= MINUTE;
		}
		if (t != 0 || tx == 0 || prec != 0)
			p += sprintf(p, "%jds", t);
	} else  {
		p += sprintf(p, "%jd", (intmax_t)t);
	}
	if (prec != 0) {
		for (i = prec; i < 9; i++)
			nsec /= 10;
		p += sprintf(p, ".%.*d", prec, nsec);
	}
	ret = __printf_out(io, pi, buf, p - buf);
	__printf_flush(io);
	return (ret);
}