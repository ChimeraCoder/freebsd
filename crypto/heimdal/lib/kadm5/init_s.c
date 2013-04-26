
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


static kadm5_ret_t
kadm5_s_init_with_context(krb5_context context,
			  const char *client_name,
			  const char *service_name,
			  kadm5_config_params *realm_params,
			  unsigned long struct_version,
			  unsigned long api_version,
			  void **server_handle)
{
    kadm5_ret_t ret;
    kadm5_server_context *ctx;
    ret = _kadm5_s_init_context(&ctx, realm_params, context);
    if(ret)
	return ret;

    assert(ctx->config.dbname != NULL);
    assert(ctx->config.stash_file != NULL);
    assert(ctx->config.acl_file != NULL);
    assert(ctx->log_context.log_file != NULL);
#ifndef NO_UNIX_SOCKETS
    assert(ctx->log_context.socket_name.sun_path[0] != '\0');
#else
    assert(ctx->log_context.socket_info != NULL);
#endif

    ret = hdb_create(ctx->context, &ctx->db, ctx->config.dbname);
    if(ret)
	return ret;
    ret = hdb_set_master_keyfile (ctx->context,
				  ctx->db, ctx->config.stash_file);
    if(ret)
	return ret;

    ctx->log_context.log_fd   = -1;

#ifndef NO_UNIX_SOCKETS
    ctx->log_context.socket_fd = socket (AF_UNIX, SOCK_DGRAM, 0);
#else
    ctx->log_context.socket_fd = socket (ctx->log_context.socket_info->ai_family,
					 ctx->log_context.socket_info->ai_socktype,
					 ctx->log_context.socket_info->ai_protocol);
#endif

    ret = krb5_parse_name(ctx->context, client_name, &ctx->caller);
    if(ret)
	return ret;

    ret = _kadm5_acl_init(ctx);
    if(ret)
	return ret;

    *server_handle = ctx;
    return 0;
}

kadm5_ret_t
kadm5_s_init_with_password_ctx(krb5_context context,
			       const char *client_name,
			       const char *password,
			       const char *service_name,
			       kadm5_config_params *realm_params,
			       unsigned long struct_version,
			       unsigned long api_version,
			       void **server_handle)
{
    return kadm5_s_init_with_context(context,
				     client_name,
				     service_name,
				     realm_params,
				     struct_version,
				     api_version,
				     server_handle);
}

kadm5_ret_t
kadm5_s_init_with_password(const char *client_name,
			   const char *password,
			   const char *service_name,
			   kadm5_config_params *realm_params,
			   unsigned long struct_version,
			   unsigned long api_version,
			   void **server_handle)
{
    krb5_context context;
    kadm5_ret_t ret;
    kadm5_server_context *ctx;

    ret = krb5_init_context(&context);
    if (ret)
	return ret;
    ret = kadm5_s_init_with_password_ctx(context,
					 client_name,
					 password,
					 service_name,
					 realm_params,
					 struct_version,
					 api_version,
					 server_handle);
    if(ret){
	krb5_free_context(context);
	return ret;
    }
    ctx = *server_handle;
    ctx->my_context = 1;
    return 0;
}

kadm5_ret_t
kadm5_s_init_with_skey_ctx(krb5_context context,
			   const char *client_name,
			   const char *keytab,
			   const char *service_name,
			   kadm5_config_params *realm_params,
			   unsigned long struct_version,
			   unsigned long api_version,
			   void **server_handle)
{
    return kadm5_s_init_with_context(context,
				     client_name,
				     service_name,
				     realm_params,
				     struct_version,
				     api_version,
				     server_handle);
}

kadm5_ret_t
kadm5_s_init_with_skey(const char *client_name,
		       const char *keytab,
		       const char *service_name,
		       kadm5_config_params *realm_params,
		       unsigned long struct_version,
		       unsigned long api_version,
		       void **server_handle)
{
    krb5_context context;
    kadm5_ret_t ret;
    kadm5_server_context *ctx;

    ret = krb5_init_context(&context);
    if (ret)
	return ret;
    ret = kadm5_s_init_with_skey_ctx(context,
				     client_name,
				     keytab,
				     service_name,
				     realm_params,
				     struct_version,
				     api_version,
				     server_handle);
    if(ret){
	krb5_free_context(context);
	return ret;
    }
    ctx = *server_handle;
    ctx->my_context = 1;
    return 0;
}

kadm5_ret_t
kadm5_s_init_with_creds_ctx(krb5_context context,
			    const char *client_name,
			    krb5_ccache ccache,
			    const char *service_name,
			    kadm5_config_params *realm_params,
			    unsigned long struct_version,
			    unsigned long api_version,
			    void **server_handle)
{
    return kadm5_s_init_with_context(context,
				     client_name,
				     service_name,
				     realm_params,
				     struct_version,
				     api_version,
				     server_handle);
}

kadm5_ret_t
kadm5_s_init_with_creds(const char *client_name,
			krb5_ccache ccache,
			const char *service_name,
			kadm5_config_params *realm_params,
			unsigned long struct_version,
			unsigned long api_version,
			void **server_handle)
{
    krb5_context context;
    kadm5_ret_t ret;
    kadm5_server_context *ctx;

    ret = krb5_init_context(&context);
    if (ret)
	return ret;
    ret = kadm5_s_init_with_creds_ctx(context,
				      client_name,
				      ccache,
				      service_name,
				      realm_params,
				      struct_version,
				      api_version,
				      server_handle);
    if(ret){
	krb5_free_context(context);
	return ret;
    }
    ctx = *server_handle;
    ctx->my_context = 1;
    return 0;
}