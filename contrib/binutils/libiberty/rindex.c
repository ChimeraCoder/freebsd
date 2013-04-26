
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
/* Stub implementation of (obsolete) rindex(). */
/*

@deftypefn Supplemental char* rindex (const char *@var{s}, int @var{c})

Returns a pointer to the last occurrence of the character @var{c} in
the string @var{s}, or @code{NULL} if not found.  The use of @code{rindex} is
deprecated in new programs in favor of @code{strrchr}.

@end deftypefn

*/

extern char *strrchr (const char *, int);

char *
rindex (const char *s, int c)
{
  return strrchr (s, c);
}