
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
@deftypefn Supplemental int gettimeofday (struct timeval *@var{tp}, void *@var{tz})

Writes the current time to @var{tp}.  This implementation requires
that @var{tz} be NULL.  Returns 0 on success, -1 on failure.

@end deftypefn

*/ 

int
gettimeofday (struct timeval *tp, void *tz)
{
  if (tz)
    abort ();
  tp->tv_usec = 0;
  if (time (&tp->tv_sec) == (time_t) -1)
    return -1;
  return 0;
}