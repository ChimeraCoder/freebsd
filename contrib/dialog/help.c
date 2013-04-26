
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

#include <dialog.h>

/*
 * Display a help-file as a textbox widget.
 */
int
dialog_helpfile(const char *title,
		const char *file,
		int height,
		int width)
{
    int result = DLG_EXIT_ERROR;

    if (!dialog_vars.in_helpfile && file != 0 && *file != '\0') {
	dialog_vars.in_helpfile = TRUE;
	result = dialog_textbox(title, file, height, width);
	dialog_vars.in_helpfile = FALSE;
    }
    return (result);
}