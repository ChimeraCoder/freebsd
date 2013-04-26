
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
/*   SYNOPSIS
       #include <string.h>

       char *strchr(char const *s, int c);

       char *strrchr(char const *s, int c);

   DESCRIPTION
       The  strchr() function returns a pointer to the first occurrence of the
       character c in the string s.

       The strrchr() function returns a pointer to the last occurrence of  the
       character c in the string s.

       Here  "character"  means "byte" - these functions do not work with wide
       or multi-byte characters.

   RETURN VALUE
       The strchr() and strrchr() functions return a pointer  to  the  matched
       character or NULL if the character is not found.

   CONFORMING TO
       SVID 3, POSIX, BSD 4.3, ISO 9899
*/

char*
strchr( char const *s, int c)
{
    do {
        if ((unsigned)*s == (unsigned)c)
            return s;

    } while (*(++s) != NUL);

    return NULL;
}

char*
strrchr( char const *s, int c)
{
    char const *e = s + strlen(s);

    for (;;) {
        if (--e < s)
            break;

        if ((unsigned)*e == (unsigned)c)
            return e;
    }
    return NULL;
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of compat/strsignal.c */