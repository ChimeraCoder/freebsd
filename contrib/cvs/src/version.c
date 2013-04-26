
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

#include "cvs.h"

#ifdef CLIENT_SUPPORT
#ifdef SERVER_SUPPORT
char *config_string = " (client/server)\n";
#else
char *config_string = " (client)\n";
#endif
#else
#ifdef SERVER_SUPPORT
char *config_string = " (server)\n";
#else
char *config_string = "\n";
#endif
#endif



static const char *const version_usage[] =
{
    "Usage: %s %s\n",
    NULL
};



/*
 * Output a version string for the client and server.
 *
 * This function will output the simple version number (for the '--version'
 * option) or the version numbers of the client and server (using the 'version'
 * command).
 */
int
version (argc, argv)
    int argc;
    char **argv;
{
    int err = 0;

    if (argc == -1)
	usage (version_usage);

    if (current_parsed_root && current_parsed_root->isremote)
        (void) fputs ("Client: ", stdout);

    /* Having the year here is a good idea, so people have
       some idea of how long ago their version of CVS was
       released.  */
    (void) fputs (PACKAGE_STRING, stdout);
    (void) fputs (config_string, stdout);

#ifdef CLIENT_SUPPORT
    if (current_parsed_root && current_parsed_root->isremote)
    {
	(void) fputs ("Server: ", stdout);
	start_server ();
	if (supported_request ("version"))
	    send_to_server ("version\012", 0);
	else
	{
	    send_to_server ("noop\012", 0);
	    fputs ("(unknown)\n", stdout);
	}
	err = get_responses_and_close ();
    }
#endif
    return err;
}
	