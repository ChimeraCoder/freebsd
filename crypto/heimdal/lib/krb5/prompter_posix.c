
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

#include "krb5_locl.h"

KRB5_LIB_FUNCTION int KRB5_CALLCONV
krb5_prompter_posix (krb5_context context,
		     void *data,
		     const char *name,
		     const char *banner,
		     int num_prompts,
		     krb5_prompt prompts[])
{
    int i;

    if (name)
	fprintf (stderr, "%s\n", name);
    if (banner)
	fprintf (stderr, "%s\n", banner);
    if (name || banner)
	fflush(stderr);
    for (i = 0; i < num_prompts; ++i) {
	if (prompts[i].hidden) {
	    if(UI_UTIL_read_pw_string(prompts[i].reply->data,
				  prompts[i].reply->length,
				  prompts[i].prompt,
				  0))
	       return 1;
	} else {
	    char *s = prompts[i].reply->data;

	    fputs (prompts[i].prompt, stdout);
	    fflush (stdout);
	    if(fgets(prompts[i].reply->data,
		     prompts[i].reply->length,
		     stdin) == NULL)
		return 1;
	    s[strcspn(s, "\n")] = '\0';
	}
    }
    return 0;
}