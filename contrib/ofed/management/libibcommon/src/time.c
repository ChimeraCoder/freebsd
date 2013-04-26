
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

#include <common.h>
#include <sys/time.h>

/**
 * getcurrenttime: Returns micro seconds elapsed from epoch.
 */
uint64_t
getcurrenttime(void)
{
        struct timeval tv;

        gettimeofday(&tv, 0);
        return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}