
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
  * Some systems do not have setenv(). This one is modeled after 4.4 BSD, but
  * is implemented in terms of portable primitives only: getenv(), putenv()
  * and malloc(). It should therefore be safe to use on every UNIX system.
  * 
  * If clobber == 0, do not overwrite an existing variable.
  * 
  * Returns nonzero if memory allocation fails.
  * 
  * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
  */
#ifndef lint
static char sccsid[] = "@(#) setenv.c 1.1 93/03/07 22:47:58";
#endif

/* setenv - update or insert environment (name,value) pair */

int     setenv(name, value, clobber)
char   *name;
char   *value;
int     clobber;
{
    char   *malloc();
    char   *getenv();
    char   *cp;

    if (clobber == 0 && getenv(name) != 0)
	return (0);
    if ((cp = malloc(strlen(name) + strlen(value) + 2)) == 0)
	return (1);
    sprintf(cp, "%s=%s", name, value);
    return (putenv(cp));
}