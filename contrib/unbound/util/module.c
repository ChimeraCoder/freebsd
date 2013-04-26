
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

#include "config.h"
#include "util/module.h"

const char* 
strextstate(enum module_ext_state s)
{
	switch(s) {
	case module_state_initial: return "module_state_initial";
	case module_wait_reply: return "module_wait_reply";
	case module_wait_module: return "module_wait_module";
	case module_restart_next: return "module_restart_next";
	case module_wait_subquery: return "module_wait_subquery";
	case module_error: return "module_error";
	case module_finished: return "module_finished";
	}
	return "bad_extstate_value";
}

const char* 
strmodulevent(enum module_ev e)
{
	switch(e) {
	case module_event_new: return "module_event_new";
	case module_event_pass: return "module_event_pass";
	case module_event_reply: return "module_event_reply";
	case module_event_noreply: return "module_event_noreply";
	case module_event_capsfail: return "module_event_capsfail";
	case module_event_moddone: return "module_event_moddone";
	case module_event_error: return "module_event_error";
	}
	return "bad_event_value";
}