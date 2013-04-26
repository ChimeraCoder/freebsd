
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

#include "kadm5_locl.h"

RCSID("$Id$");

kadm5_ret_t
kadm5_init_with_password(const char *client_name,
			 const char *password,
			 const char *service_name,
			 kadm5_config_params *realm_params,
			 unsigned long struct_version,
			 unsigned long api_version,
			 void **server_handle)
{
    return kadm5_c_init_with_password(client_name,
				      password,
				      service_name,
				      realm_params,
				      struct_version,
				      api_version,
				      server_handle);
}

kadm5_ret_t
kadm5_init_with_password_ctx(krb5_context context,
			     const char *client_name,
			     const char *password,
			     const char *service_name,
			     kadm5_config_params *realm_params,
			     unsigned long struct_version,
			     unsigned long api_version,
			     void **server_handle)
{
    return kadm5_c_init_with_password_ctx(context,
					  client_name,
					  password,
					  service_name,
					  realm_params,
					  struct_version,
					  api_version,
					  server_handle);
}

kadm5_ret_t
kadm5_init_with_skey(const char *client_name,
		     const char *keytab,
		     const char *service_name,
		     kadm5_config_params *realm_params,
		     unsigned long struct_version,
		     unsigned long api_version,
		     void **server_handle)
{
    return kadm5_c_init_with_skey(client_name,
				  keytab,
				  service_name,
				  realm_params,
				  struct_version,
				  api_version,
				  server_handle);
}

kadm5_ret_t
kadm5_init_with_skey_ctx(krb5_context context,
			 const char *client_name,
			 const char *keytab,
			 const char *service_name,
			 kadm5_config_params *realm_params,
			 unsigned long struct_version,
			 unsigned long api_version,
			 void **server_handle)
{
    return kadm5_c_init_with_skey_ctx(context,
				      client_name,
				      keytab,
				      service_name,
				      realm_params,
				      struct_version,
				      api_version,
				      server_handle);
}

kadm5_ret_t
kadm5_init_with_creds(const char *client_name,
		      krb5_ccache ccache,
		      const char *service_name,
		      kadm5_config_params *realm_params,
		      unsigned long struct_version,
		      unsigned long api_version,
		      void **server_handle)
{
    return kadm5_c_init_with_creds(client_name,
				   ccache,
				   service_name,
				   realm_params,
				   struct_version,
				   api_version,
				   server_handle);
}

kadm5_ret_t
kadm5_init_with_creds_ctx(krb5_context context,
			  const char *client_name,
			  krb5_ccache ccache,
			  const char *service_name,
			  kadm5_config_params *realm_params,
			  unsigned long struct_version,
			  unsigned long api_version,
			  void **server_handle)
{
    return kadm5_c_init_with_creds_ctx(context,
				       client_name,
				       ccache,
				       service_name,
				       realm_params,
				       struct_version,
				       api_version,
				       server_handle);
}