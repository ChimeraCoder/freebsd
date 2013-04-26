
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

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_appdefault_boolean(krb5_context context, const char *appname,
			krb5_const_realm realm, const char *option,
			krb5_boolean def_val, krb5_boolean *ret_val)
{

    if(appname == NULL)
	appname = getprogname();

    def_val = krb5_config_get_bool_default(context, NULL, def_val,
					   "libdefaults", option, NULL);
    if(realm != NULL)
	def_val = krb5_config_get_bool_default(context, NULL, def_val,
					       "realms", realm, option, NULL);

    def_val = krb5_config_get_bool_default(context, NULL, def_val,
					   "appdefaults",
					   option,
					   NULL);
    if(realm != NULL)
	def_val = krb5_config_get_bool_default(context, NULL, def_val,
					       "appdefaults",
					       realm,
					       option,
					       NULL);
    if(appname != NULL) {
	def_val = krb5_config_get_bool_default(context, NULL, def_val,
					       "appdefaults",
					       appname,
					       option,
					       NULL);
	if(realm != NULL)
	    def_val = krb5_config_get_bool_default(context, NULL, def_val,
						   "appdefaults",
						   appname,
						   realm,
						   option,
						   NULL);
    }
    *ret_val = def_val;
}

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_appdefault_string(krb5_context context, const char *appname,
		       krb5_const_realm realm, const char *option,
		       const char *def_val, char **ret_val)
{
    if(appname == NULL)
	appname = getprogname();

    def_val = krb5_config_get_string_default(context, NULL, def_val,
					     "libdefaults", option, NULL);
    if(realm != NULL)
	def_val = krb5_config_get_string_default(context, NULL, def_val,
						 "realms", realm, option, NULL);

    def_val = krb5_config_get_string_default(context, NULL, def_val,
					     "appdefaults",
					     option,
					     NULL);
    if(realm != NULL)
	def_val = krb5_config_get_string_default(context, NULL, def_val,
						 "appdefaults",
						 realm,
						 option,
						 NULL);
    if(appname != NULL) {
	def_val = krb5_config_get_string_default(context, NULL, def_val,
						 "appdefaults",
						 appname,
						 option,
						 NULL);
	if(realm != NULL)
	    def_val = krb5_config_get_string_default(context, NULL, def_val,
						     "appdefaults",
						     appname,
						     realm,
						     option,
						     NULL);
    }
    if(def_val != NULL)
	*ret_val = strdup(def_val);
    else
	*ret_val = NULL;
}

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_appdefault_time(krb5_context context, const char *appname,
		     krb5_const_realm realm, const char *option,
		     time_t def_val, time_t *ret_val)
{
    krb5_deltat t;
    char *val;

    krb5_appdefault_string(context, appname, realm, option, NULL, &val);
    if (val == NULL) {
	*ret_val = def_val;
	return;
    }
    if (krb5_string_to_deltat(val, &t))
	*ret_val = def_val;
    else
	*ret_val = t;
    free(val);
}