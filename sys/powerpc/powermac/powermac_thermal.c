
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

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/systm.h>

#include <sys/types.h>
#include <sys/kthread.h>
#include <sys/malloc.h>
#include <sys/reboot.h>
#include <sys/sysctl.h>
#include <sys/queue.h>

#include "powermac_thermal.h"

static void fan_management_proc(void);
static void pmac_therm_manage_fans(void);

static struct proc *pmac_them_proc;
static int enable_pmac_thermal = 1;

static struct kproc_desc pmac_therm_kp = {
	"pmac_thermal",
	fan_management_proc,
	&pmac_them_proc
};

SYSINIT(pmac_therm_setup, SI_SUB_KTHREAD_IDLE, SI_ORDER_ANY, kproc_start,
    &pmac_therm_kp);
SYSCTL_INT(_machdep, OID_AUTO, manage_fans, CTLFLAG_RW | CTLFLAG_TUN,
    &enable_pmac_thermal, 1, "Enable automatic fan management");
static MALLOC_DEFINE(M_PMACTHERM, "pmactherm", "Powermac Thermal Management");

struct pmac_fan_le {
	struct pmac_fan			*fan;
	int				last_val;
	SLIST_ENTRY(pmac_fan_le)	entries;
};
struct pmac_sens_le {
	struct pmac_therm		*sensor;
	int				last_val;
	SLIST_ENTRY(pmac_sens_le)	entries;
};
static SLIST_HEAD(pmac_fans, pmac_fan_le) fans = SLIST_HEAD_INITIALIZER(fans);
static SLIST_HEAD(pmac_sensors, pmac_sens_le) sensors =
    SLIST_HEAD_INITIALIZER(sensors);

static void
fan_management_proc(void)
{
	/* Nothing to manage? */
	if (SLIST_EMPTY(&fans))
		kproc_exit(0);
	
	while (1) {
		pmac_therm_manage_fans();
		pause("pmac_therm", hz);
	}
}

static void
pmac_therm_manage_fans(void)
{
	struct pmac_sens_le *sensor;
	struct pmac_fan_le *fan;
	int average_excess, max_excess_zone, frac_excess;
	int nsens, nsens_zone;
	int temp;

	if (!enable_pmac_thermal)
		return;

	/* Read all the sensors */
	SLIST_FOREACH(sensor, &sensors, entries) {
		temp = sensor->sensor->read(sensor->sensor);
		if (temp > 0) /* Use the previous temp in case of error */
			sensor->last_val = temp;

		if (sensor->last_val > sensor->sensor->max_temp) {
			printf("WARNING: Current temperature (%s: %d.%d C) "
			    "exceeds critical temperature (%d.%d C)! "
			    "Shutting down!\n", sensor->sensor->name,
			       (sensor->last_val - ZERO_C_TO_K) / 10,
			       (sensor->last_val - ZERO_C_TO_K) % 10,
			       (sensor->sensor->max_temp - ZERO_C_TO_K) / 10,
			       (sensor->sensor->max_temp - ZERO_C_TO_K) % 10);
			shutdown_nice(RB_POWEROFF);
		}
	}

	/* Set all the fans */
	SLIST_FOREACH(fan, &fans, entries) {
		nsens = nsens_zone = 0;
		average_excess = max_excess_zone = 0;
		SLIST_FOREACH(sensor, &sensors, entries) {
			frac_excess = (sensor->last_val -
			    sensor->sensor->target_temp)*100 /
			    (sensor->sensor->max_temp -
			    sensor->sensor->target_temp);
			if (frac_excess < 0)
				frac_excess = 0;
			if (sensor->sensor->zone == fan->fan->zone) {
				max_excess_zone = imax(max_excess_zone,
				    frac_excess);
				nsens_zone++;
			}
			average_excess += frac_excess;
			nsens++;
		}
		average_excess /= nsens;

		/* If there are no sensors in this zone, use the average */
		if (nsens_zone == 0)
			max_excess_zone = average_excess;
		/* No sensors at all? Use default */
		if (nsens == 0) {
			fan->fan->set(fan->fan, fan->fan->default_rpm);
			continue;
		}

		/*
		 * Scale the fan linearly in the max temperature in its
		 * thermal zone.
		 */
		fan->fan->set(fan->fan, max_excess_zone *
		    (fan->fan->max_rpm - fan->fan->min_rpm)/100 +
		    fan->fan->min_rpm);
	}
}

void
pmac_thermal_fan_register(struct pmac_fan *fan)
{
	struct pmac_fan_le *list_entry;

	list_entry = malloc(sizeof(struct pmac_fan_le), M_PMACTHERM,
	    M_ZERO | M_WAITOK);
	list_entry->fan = fan;

	SLIST_INSERT_HEAD(&fans, list_entry, entries);
}

void
pmac_thermal_sensor_register(struct pmac_therm *sensor)
{
	struct pmac_sens_le *list_entry;

	list_entry = malloc(sizeof(struct pmac_sens_le), M_PMACTHERM,
	    M_ZERO | M_WAITOK);
	list_entry->sensor = sensor;

	SLIST_INSERT_HEAD(&sensors, list_entry, entries);
}