
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
 * DTrace Anonymous Enabling Functions
 */
static void
dtrace_anon_init(void *dummy)
{
	dtrace_state_t *state = NULL;
	dtrace_enabling_t *enab;

	mutex_enter(&cpu_lock);
	mutex_enter(&dtrace_provider_lock);
	mutex_enter(&dtrace_lock);

	dtrace_anon_property();

	mutex_exit(&cpu_lock);

	/*
	 * If there are already providers, we must ask them to provide their
	 * probes, and then match any anonymous enabling against them.  Note
	 * that there should be no other retained enablings at this time:
	 * the only retained enablings at this time should be the anonymous
	 * enabling.
	 */
	if (dtrace_anon.dta_enabling != NULL) {
		ASSERT(dtrace_retained == dtrace_anon.dta_enabling);

		dtrace_enabling_provide(NULL);
		state = dtrace_anon.dta_state;

		/*
		 * We couldn't hold cpu_lock across the above call to
		 * dtrace_enabling_provide(), but we must hold it to actually
		 * enable the probes.  We have to drop all of our locks, pick
		 * up cpu_lock, and regain our locks before matching the
		 * retained anonymous enabling.
		 */
		mutex_exit(&dtrace_lock);
		mutex_exit(&dtrace_provider_lock);

		mutex_enter(&cpu_lock);
		mutex_enter(&dtrace_provider_lock);
		mutex_enter(&dtrace_lock);

		if ((enab = dtrace_anon.dta_enabling) != NULL)
			(void) dtrace_enabling_match(enab, NULL);

		mutex_exit(&cpu_lock);
	}

	mutex_exit(&dtrace_provider_lock);
	mutex_exit(&dtrace_lock);

	if (state != NULL) {
		/*
		 * If we created any anonymous state, set it going now.
		 */
		(void) dtrace_state_go(state, &dtrace_anon.dta_beganon);
	}
}