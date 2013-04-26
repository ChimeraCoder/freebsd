
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

/* $Id: timer.c,v 1.7 2007/06/19 23:47:16 tbox Exp $ */

/*! \file */

#include <config.h>

#include <isc/result.h>
#include <isc/time.h>
#include <isc/timer.h>

#include <dns/types.h>
#include <dns/timer.h>

#define CHECK(op) \
	do { result = (op);					\
		if (result != ISC_R_SUCCESS) goto failure;	\
	} while (0)

isc_result_t
dns_timer_setidle(isc_timer_t *timer, unsigned int maxtime,
		  unsigned int idletime, isc_boolean_t purge)
{
	isc_result_t result;
	isc_interval_t maxinterval, idleinterval;
	isc_time_t expires;

	/* Compute the time of expiry. */
	isc_interval_set(&maxinterval, maxtime, 0);
	CHECK(isc_time_nowplusinterval(&expires, &maxinterval));

	/*
	 * Compute the idle interval, and add a spare nanosecond to
	 * work around the silly limitation of the ISC timer interface
	 * that you cannot specify an idle interval of zero.
	 */
	isc_interval_set(&idleinterval, idletime, 1);

	CHECK(isc_timer_reset(timer, isc_timertype_once,
			      &expires, &idleinterval,
			      purge));
 failure:
	return (result);
}