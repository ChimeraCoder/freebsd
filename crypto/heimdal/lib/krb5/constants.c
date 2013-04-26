
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

KRB5_LIB_VARIABLE const char *krb5_config_file =
#ifdef __APPLE__
"~/Library/Preferences/com.apple.Kerberos.plist" PATH_SEP
"/Library/Preferences/com.apple.Kerberos.plist" PATH_SEP
"~/Library/Preferences/edu.mit.Kerberos" PATH_SEP
"/Library/Preferences/edu.mit.Kerberos" PATH_SEP
#endif	/* __APPLE__ */
"~/.krb5/config" PATH_SEP
SYSCONFDIR "/krb5.conf"
#ifdef _WIN32
PATH_SEP "%{COMMON_APPDATA}/Kerberos/krb5.conf"
PATH_SEP "%{WINDOWS}/krb5.ini"
#else
PATH_SEP "/etc/krb5.conf"
#endif
;

KRB5_LIB_VARIABLE const char *krb5_defkeyname = KEYTAB_DEFAULT;

KRB5_LIB_VARIABLE const char *krb5_cc_type_api = "API";
KRB5_LIB_VARIABLE const char *krb5_cc_type_file = "FILE";
KRB5_LIB_VARIABLE const char *krb5_cc_type_memory = "MEMORY";
KRB5_LIB_VARIABLE const char *krb5_cc_type_kcm = "KCM";
KRB5_LIB_VARIABLE const char *krb5_cc_type_scc = "SCC";