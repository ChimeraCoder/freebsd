
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
#include <sys/systm.h>
#include <sys/proc.h>

#include <sys/power.h>
#include <sys/taskqueue.h>

static u_int		 power_pm_type	= POWER_PM_TYPE_NONE;
static power_pm_fn_t	 power_pm_fn	= NULL;
static void		*power_pm_arg	= NULL;
static struct task	 power_pm_task;

static void
power_pm_deferred_fn(void *arg, int pending)
{
	int state = (intptr_t)arg;

	power_pm_fn(POWER_CMD_SUSPEND, power_pm_arg, state);
}

int
power_pm_register(u_int pm_type, power_pm_fn_t pm_fn, void *pm_arg)
{
	int	error;

	if (power_pm_type == POWER_PM_TYPE_NONE ||
	    power_pm_type == pm_type) {
		power_pm_type	= pm_type;
		power_pm_fn	= pm_fn;
		power_pm_arg	= pm_arg;
		error = 0;
		TASK_INIT(&power_pm_task, 0, power_pm_deferred_fn, NULL);
	} else {
		error = ENXIO;
	}

	return (error);
}

u_int
power_pm_get_type(void)
{

	return (power_pm_type);
}

void
power_pm_suspend(int state)
{
	if (power_pm_fn == NULL)
		return;

	if (state != POWER_SLEEP_STATE_STANDBY &&
	    state != POWER_SLEEP_STATE_SUSPEND &&
	    state != POWER_SLEEP_STATE_HIBERNATE)
		return;
	power_pm_task.ta_context = (void *)(intptr_t)state;
	taskqueue_enqueue(taskqueue_thread, &power_pm_task);
}

/*
 * Power profile.
 */

static int	power_profile_state = POWER_PROFILE_PERFORMANCE;

int
power_profile_get_state(void)
{
	return (power_profile_state);
}

void
power_profile_set_state(int state) 
{
	int		changed;
    
	if (state != power_profile_state) {
		power_profile_state = state;
		changed = 1;
		if (bootverbose) {
			printf("system power profile changed to '%s'\n",
				(state == POWER_PROFILE_PERFORMANCE) ?
				"performance" : "economy");
		}
	} else {
		changed = 0;
	}

	if (changed)
		EVENTHANDLER_INVOKE(power_profile_change, 0);
}