
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

#include "thr_private.h"

void
_thread_bp_create(void)
{
}

void
_thread_bp_death(void)
{
}

void
_thr_report_creation(struct pthread *curthread, struct pthread *newthread)
{
	curthread->event_buf.event = TD_CREATE;
	curthread->event_buf.th_p = (uintptr_t)newthread;
	curthread->event_buf.data = 0;
	THR_UMUTEX_LOCK(curthread, &_thr_event_lock);
	_thread_last_event = curthread;
	_thread_bp_create();
	_thread_last_event = NULL;
	THR_UMUTEX_UNLOCK(curthread, &_thr_event_lock);
}

void
_thr_report_death(struct pthread *curthread)
{
	curthread->event_buf.event = TD_DEATH;
	curthread->event_buf.th_p = (uintptr_t)curthread;
	curthread->event_buf.data = 0;
	THR_UMUTEX_LOCK(curthread, &_thr_event_lock);
	_thread_last_event = curthread;
	_thread_bp_death();
	_thread_last_event = NULL;
	THR_UMUTEX_UNLOCK(curthread, &_thr_event_lock);
}