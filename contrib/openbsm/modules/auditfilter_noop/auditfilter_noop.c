
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
 * Sample audit filter: no-op which sinks audit records in both BSM and
 * parsed formats, but does nothing with them.
 */

#include <bsm/libbsm.h>

#include <bsm/libbsm.h>
#include <bsm/audit_filter.h>

#ifndef __unused
#define __unused
#endif

int
AUDIT_FILTER_ATTACH(void *instance __unused, int argc __unused, 
    char *argv[] __unused)
{

	return (0);
}

int
AUDIT_FILTER_REINIT(void *instance __unused, int argc __unused, 
    char *argv[] __unused)
{

	return (0);
}

void
AUDIT_FILTER_RECORD(void *instance __unused, struct timespec *ts __unused,
    int token_count __unused, const tokenstr_t *tok[] __unused)
{

}

void
AUDIT_FILTER_RAWRECORD(void *instance __unused, struct timespec *ts __unused,
    u_char *data __unused, u_int len __unused)
{

}

void
AUDIT_FILTER_DETACH(void *instance __unused)
{

}