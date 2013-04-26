
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
/*******************************************************************************                                                                             *
*   s t r s t r                                                               *
*                                                                             *
*   Find the first occurrence of a string in another string.                  *
*                                                                             *
* Format:                                                                     *
*             return = strstr(Source,What);                                   *
*                                                                             *
* Parameters:                                                                 *
*                                                                             *
* Returns:                                                                    *
*                                                                             *
* Scope:      PUBLIC                                                          *
*                                                                             *
******************************************************************************/

char *strstr(Source, What)
register const char *Source;
register const char *What;
{
register char WhatChar;
register char SourceChar;
register long Length;


    if ((WhatChar = *What++) != 0) {
        Length = strlen(What);
        do {
            do {
                if ((SourceChar = *Source++) == 0) {
                    return (0);
                }
            } while (SourceChar != WhatChar);
        } while (strncmp(Source, What, Length) != 0);
        Source--;
    }
    return ((char *)Source);

}/*strstr*/