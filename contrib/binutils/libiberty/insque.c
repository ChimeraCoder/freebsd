
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
/* insque(3C) routines   This file is in the public domain.  */

/*

@deftypefn Supplemental void insque (struct qelem *@var{elem}, struct qelem *@var{pred})
@deftypefnx Supplemental void remque (struct qelem *@var{elem})

Routines to manipulate queues built from doubly linked lists.  The
@code{insque} routine inserts @var{elem} in the queue immediately
after @var{pred}.  The @code{remque} routine removes @var{elem} from
its containing queue.  These routines expect to be passed pointers to
structures which have as their first members a forward pointer and a
back pointer, like this prototype (although no prototype is provided):

@example
struct qelem @{
  struct qelem *q_forw;
  struct qelem *q_back;
  char q_data[];
@};
@end example

@end deftypefn

*/


struct qelem {
  struct qelem *q_forw;
  struct qelem *q_back;
};


void
insque (struct qelem *elem, struct qelem *pred)
{
  elem -> q_forw = pred -> q_forw;
  pred -> q_forw -> q_back = elem;
  elem -> q_back = pred;
  pred -> q_forw = elem;
}


void
remque (struct qelem *elem)
{
  elem -> q_forw -> q_back = elem -> q_back;
  elem -> q_back -> q_forw = elem -> q_forw;
}