
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
#include "ssl_locl.h"
#ifndef OPENSSL_NO_SRP

#include <openssl/rand.h>
#include <openssl/srp.h>
#include <openssl/err.h>

int SSL_CTX_SRP_CTX_free(struct ssl_ctx_st *ctx)
	{
	if (ctx == NULL)
		return 0;
	OPENSSL_free(ctx->srp_ctx.login);
	BN_free(ctx->srp_ctx.N);
	BN_free(ctx->srp_ctx.g);
	BN_free(ctx->srp_ctx.s);
	BN_free(ctx->srp_ctx.B);
	BN_free(ctx->srp_ctx.A);
	BN_free(ctx->srp_ctx.a);
	BN_free(ctx->srp_ctx.b);
	BN_free(ctx->srp_ctx.v);
	ctx->srp_ctx.TLS_ext_srp_username_callback = NULL;
	ctx->srp_ctx.SRP_cb_arg = NULL;
	ctx->srp_ctx.SRP_verify_param_callback = NULL;
	ctx->srp_ctx.SRP_give_srp_client_pwd_callback = NULL;
	ctx->srp_ctx.N = NULL;
	ctx->srp_ctx.g = NULL;
	ctx->srp_ctx.s = NULL;
	ctx->srp_ctx.B = NULL;
	ctx->srp_ctx.A = NULL;
	ctx->srp_ctx.a = NULL;
	ctx->srp_ctx.b = NULL;
	ctx->srp_ctx.v = NULL;
	ctx->srp_ctx.login = NULL;
	ctx->srp_ctx.info = NULL;
	ctx->srp_ctx.strength = SRP_MINIMAL_N;
	ctx->srp_ctx.srp_Mask = 0;
	return (1);
	}

int SSL_SRP_CTX_free(struct ssl_st *s)
	{
	if (s == NULL)
		return 0;
	OPENSSL_free(s->srp_ctx.login);
	BN_free(s->srp_ctx.N);
	BN_free(s->srp_ctx.g);
	BN_free(s->srp_ctx.s);
	BN_free(s->srp_ctx.B);
	BN_free(s->srp_ctx.A);
	BN_free(s->srp_ctx.a);
	BN_free(s->srp_ctx.b);
	BN_free(s->srp_ctx.v);
	s->srp_ctx.TLS_ext_srp_username_callback = NULL;
	s->srp_ctx.SRP_cb_arg = NULL;
	s->srp_ctx.SRP_verify_param_callback = NULL;
	s->srp_ctx.SRP_give_srp_client_pwd_callback = NULL;
	s->srp_ctx.N = NULL;
	s->srp_ctx.g = NULL;
	s->srp_ctx.s = NULL;
	s->srp_ctx.B = NULL;
	s->srp_ctx.A = NULL;
	s->srp_ctx.a = NULL;
	s->srp_ctx.b = NULL;
	s->srp_ctx.v = NULL;
	s->srp_ctx.login = NULL;
	s->srp_ctx.info = NULL;
	s->srp_ctx.strength = SRP_MINIMAL_N;
	s->srp_ctx.srp_Mask = 0;
	return (1);
	}

int SSL_SRP_CTX_init(struct ssl_st *s)
	{
	SSL_CTX *ctx;

	if ((s == NULL) || ((ctx = s->ctx) == NULL))
		return 0;
	s->srp_ctx.SRP_cb_arg = ctx->srp_ctx.SRP_cb_arg;
	/* set client Hello login callback */
	s->srp_ctx.TLS_ext_srp_username_callback = ctx->srp_ctx.TLS_ext_srp_username_callback;
	/* set SRP N/g param callback for verification */
	s->srp_ctx.SRP_verify_param_callback = ctx->srp_ctx.SRP_verify_param_callback;
	/* set SRP client passwd callback */
	s->srp_ctx.SRP_give_srp_client_pwd_callback = ctx->srp_ctx.SRP_give_srp_client_pwd_callback;

	s->srp_ctx.N = NULL;
	s->srp_ctx.g = NULL;
	s->srp_ctx.s = NULL;
	s->srp_ctx.B = NULL;
	s->srp_ctx.A = NULL;
	s->srp_ctx.a = NULL;
	s->srp_ctx.b = NULL;
	s->srp_ctx.v = NULL;
	s->srp_ctx.login = NULL;
	s->srp_ctx.info = ctx->srp_ctx.info;
	s->srp_ctx.strength = ctx->srp_ctx.strength;

	if (((ctx->srp_ctx.N != NULL) &&
		 ((s->srp_ctx.N = BN_dup(ctx->srp_ctx.N)) == NULL)) ||
		((ctx->srp_ctx.g != NULL) &&
		 ((s->srp_ctx.g = BN_dup(ctx->srp_ctx.g)) == NULL)) ||
		((ctx->srp_ctx.s != NULL) &&
		 ((s->srp_ctx.s = BN_dup(ctx->srp_ctx.s)) == NULL)) ||
		((ctx->srp_ctx.B != NULL) &&
		 ((s->srp_ctx.B = BN_dup(ctx->srp_ctx.B)) == NULL)) ||
		((ctx->srp_ctx.A != NULL) &&
		 ((s->srp_ctx.A = BN_dup(ctx->srp_ctx.A)) == NULL)) ||
		((ctx->srp_ctx.a != NULL) &&
		 ((s->srp_ctx.a = BN_dup(ctx->srp_ctx.a)) == NULL)) ||
		((ctx->srp_ctx.v != NULL) &&
		 ((s->srp_ctx.v = BN_dup(ctx->srp_ctx.v)) == NULL)) ||
		((ctx->srp_ctx.b != NULL) &&
		 ((s->srp_ctx.b = BN_dup(ctx->srp_ctx.b)) == NULL)))
		{
		SSLerr(SSL_F_SSL_SRP_CTX_INIT,ERR_R_BN_LIB);
		goto err;
		}
	if ((ctx->srp_ctx.login != NULL) && 
		((s->srp_ctx.login = BUF_strdup(ctx->srp_ctx.login)) == NULL))
		{
		SSLerr(SSL_F_SSL_SRP_CTX_INIT,ERR_R_INTERNAL_ERROR);
		goto err;
		}
	s->srp_ctx.srp_Mask = ctx->srp_ctx.srp_Mask;

	return (1);
err:
	OPENSSL_free(s->srp_ctx.login);
	BN_free(s->srp_ctx.N);
	BN_free(s->srp_ctx.g);
	BN_free(s->srp_ctx.s);
	BN_free(s->srp_ctx.B);
	BN_free(s->srp_ctx.A);
	BN_free(s->srp_ctx.a);
	BN_free(s->srp_ctx.b);
	BN_free(s->srp_ctx.v);
	return (0);
	}

int SSL_CTX_SRP_CTX_init(struct ssl_ctx_st *ctx)
	{
	if (ctx == NULL)
		return 0;

	ctx->srp_ctx.SRP_cb_arg = NULL;
	/* set client Hello login callback */
	ctx->srp_ctx.TLS_ext_srp_username_callback = NULL;
	/* set SRP N/g param callback for verification */
	ctx->srp_ctx.SRP_verify_param_callback = NULL;
	/* set SRP client passwd callback */
	ctx->srp_ctx.SRP_give_srp_client_pwd_callback = NULL;

	ctx->srp_ctx.N = NULL;
	ctx->srp_ctx.g = NULL;
	ctx->srp_ctx.s = NULL;
	ctx->srp_ctx.B = NULL;
	ctx->srp_ctx.A = NULL;
	ctx->srp_ctx.a = NULL;
	ctx->srp_ctx.b = NULL;
	ctx->srp_ctx.v = NULL;
	ctx->srp_ctx.login = NULL;
	ctx->srp_ctx.srp_Mask = 0;
	ctx->srp_ctx.info = NULL;
	ctx->srp_ctx.strength = SRP_MINIMAL_N;

	return (1);
	}

/* server side */
int SSL_srp_server_param_with_username(SSL *s, int *ad)
	{
	unsigned char b[SSL_MAX_MASTER_KEY_LENGTH];
	int al;

	*ad = SSL_AD_UNKNOWN_PSK_IDENTITY;
	if ((s->srp_ctx.TLS_ext_srp_username_callback !=NULL) &&
		((al = s->srp_ctx.TLS_ext_srp_username_callback(s, ad, s->srp_ctx.SRP_cb_arg))!=SSL_ERROR_NONE))
			return al;

	*ad = SSL_AD_INTERNAL_ERROR;
	if ((s->srp_ctx.N == NULL) ||
		(s->srp_ctx.g == NULL) ||
		(s->srp_ctx.s == NULL) ||
		(s->srp_ctx.v == NULL))
		return SSL3_AL_FATAL;

	if (RAND_bytes(b, sizeof(b)) <= 0)
		return SSL3_AL_FATAL;
	s->srp_ctx.b = BN_bin2bn(b,sizeof(b),NULL);
	OPENSSL_cleanse(b,sizeof(b));

	/* Calculate:  B = (kv + g^b) % N  */

	return ((s->srp_ctx.B = SRP_Calc_B(s->srp_ctx.b, s->srp_ctx.N, s->srp_ctx.g, s->srp_ctx.v)) != NULL)?
			SSL_ERROR_NONE:SSL3_AL_FATAL;
	}

/* If the server just has the raw password, make up a verifier entry on the fly */
int SSL_set_srp_server_param_pw(SSL *s, const char *user, const char *pass, const char *grp)
	{
	SRP_gN *GN = SRP_get_default_gN(grp);
	if(GN == NULL) return -1;
	s->srp_ctx.N = BN_dup(GN->N);
	s->srp_ctx.g = BN_dup(GN->g);
	if(s->srp_ctx.v != NULL)
		{
		BN_clear_free(s->srp_ctx.v);
		s->srp_ctx.v = NULL;
		}
	if(s->srp_ctx.s != NULL)
		{
		BN_clear_free(s->srp_ctx.s);
		s->srp_ctx.s = NULL;
		}
	if(!SRP_create_verifier_BN(user, pass, &s->srp_ctx.s, &s->srp_ctx.v, GN->N, GN->g)) return -1;

	return 1;
	}

int SSL_set_srp_server_param(SSL *s, const BIGNUM *N, const BIGNUM *g,
			     BIGNUM *sa, BIGNUM *v, char *info)
	{
	if (N!= NULL)
		{
		if (s->srp_ctx.N != NULL)
			{
			if (!BN_copy(s->srp_ctx.N,N))
				{
				BN_free(s->srp_ctx.N);
				s->srp_ctx.N = NULL;
				}
			}
		else
			s->srp_ctx.N = BN_dup(N);
		}
	if (g!= NULL)
		{
		if (s->srp_ctx.g != NULL)
			{
			if (!BN_copy(s->srp_ctx.g,g))
				{
				BN_free(s->srp_ctx.g);
				s->srp_ctx.g = NULL;
				}
			}
		else
			s->srp_ctx.g = BN_dup(g);
		}
	if (sa!= NULL)
		{
		if (s->srp_ctx.s != NULL)
			{
			if (!BN_copy(s->srp_ctx.s,sa))
				{
				BN_free(s->srp_ctx.s);
				s->srp_ctx.s = NULL;
				}
			}
		else
			s->srp_ctx.s = BN_dup(sa);
		}
	if (v!= NULL)
		{
		if (s->srp_ctx.v != NULL)
			{
			if (!BN_copy(s->srp_ctx.v,v))
				{
				BN_free(s->srp_ctx.v);
				s->srp_ctx.v = NULL;
				}
			}
		else
			s->srp_ctx.v = BN_dup(v);
		}
	s->srp_ctx.info = info;

	if (!(s->srp_ctx.N) ||
		!(s->srp_ctx.g) ||
		!(s->srp_ctx.s) ||
		!(s->srp_ctx.v))
		return -1;

	return 1;
	}

int SRP_generate_server_master_secret(SSL *s,unsigned char *master_key)
	{
	BIGNUM *K = NULL, *u = NULL;
	int ret = -1, tmp_len;
	unsigned char *tmp = NULL;

	if (!SRP_Verify_A_mod_N(s->srp_ctx.A,s->srp_ctx.N))
		goto err;
	if (!(u = SRP_Calc_u(s->srp_ctx.A,s->srp_ctx.B,s->srp_ctx.N)))
		goto err;
	if (!(K = SRP_Calc_server_key(s->srp_ctx.A, s->srp_ctx.v, u, s->srp_ctx.b, s->srp_ctx.N)))
		goto err;

	tmp_len = BN_num_bytes(K);
	if ((tmp = OPENSSL_malloc(tmp_len)) == NULL)
		goto err;
	BN_bn2bin(K, tmp);
	ret = s->method->ssl3_enc->generate_master_secret(s,master_key,tmp,tmp_len);
err:
	if (tmp)
		{
		OPENSSL_cleanse(tmp,tmp_len) ;
		OPENSSL_free(tmp);
		}
	BN_clear_free(K);
	BN_clear_free(u);
	return ret;
	}

/* client side */
int SRP_generate_client_master_secret(SSL *s,unsigned char *master_key)
	{
	BIGNUM *x = NULL, *u = NULL, *K = NULL;
	int ret = -1, tmp_len;
	char *passwd = NULL;
	unsigned char *tmp = NULL;

	/* Checks if b % n == 0
	 */
	if (SRP_Verify_B_mod_N(s->srp_ctx.B,s->srp_ctx.N)==0) goto err;
	if (!(u = SRP_Calc_u(s->srp_ctx.A,s->srp_ctx.B,s->srp_ctx.N))) goto err;
	if (s->srp_ctx.SRP_give_srp_client_pwd_callback == NULL) goto err;
	if (!(passwd = s->srp_ctx.SRP_give_srp_client_pwd_callback(s, s->srp_ctx.SRP_cb_arg))) goto err;
	if (!(x = SRP_Calc_x(s->srp_ctx.s,s->srp_ctx.login,passwd))) goto err;
	if (!(K = SRP_Calc_client_key(s->srp_ctx.N, s->srp_ctx.B, s->srp_ctx.g, x, s->srp_ctx.a, u))) goto err;

	tmp_len = BN_num_bytes(K);
	if ((tmp = OPENSSL_malloc(tmp_len)) == NULL) goto err;
	BN_bn2bin(K, tmp);
	ret = s->method->ssl3_enc->generate_master_secret(s,master_key,tmp,tmp_len);
err:
	if (tmp)
		{
		OPENSSL_cleanse(tmp,tmp_len) ;
		OPENSSL_free(tmp);
		}
	BN_clear_free(K);
	BN_clear_free(x);
	if (passwd)
		{
		OPENSSL_cleanse(passwd,strlen(passwd)) ;
		OPENSSL_free(passwd);
		}
	BN_clear_free(u);
	return ret;
	}

int SRP_Calc_A_param(SSL *s)
	{
	unsigned char rnd[SSL_MAX_MASTER_KEY_LENGTH];

	if (BN_num_bits(s->srp_ctx.N) < s->srp_ctx.strength)
		return -1;

	if (s->srp_ctx.SRP_verify_param_callback ==NULL && 
		!SRP_check_known_gN_param(s->srp_ctx.g,s->srp_ctx.N))
		return -1 ;

	RAND_bytes(rnd, sizeof(rnd));
	s->srp_ctx.a = BN_bin2bn(rnd, sizeof(rnd), s->srp_ctx.a);
	OPENSSL_cleanse(rnd, sizeof(rnd));

	if (!(s->srp_ctx.A = SRP_Calc_A(s->srp_ctx.a,s->srp_ctx.N,s->srp_ctx.g)))
		return -1;

	/* We can have a callback to verify SRP param!! */
	if (s->srp_ctx.SRP_verify_param_callback !=NULL) 
		return s->srp_ctx.SRP_verify_param_callback(s,s->srp_ctx.SRP_cb_arg);

	return 1;
	}

BIGNUM *SSL_get_srp_g(SSL *s)
	{
	if (s->srp_ctx.g != NULL)
		return s->srp_ctx.g;
	return s->ctx->srp_ctx.g;
	}

BIGNUM *SSL_get_srp_N(SSL *s)
	{
	if (s->srp_ctx.N != NULL)
		return s->srp_ctx.N;
	return s->ctx->srp_ctx.N;
	}

char *SSL_get_srp_username(SSL *s)
	{
	if (s->srp_ctx.login != NULL)
		return s->srp_ctx.login;
	return s->ctx->srp_ctx.login;
	}

char *SSL_get_srp_userinfo(SSL *s)
	{
	if (s->srp_ctx.info != NULL)
		return s->srp_ctx.info;
	return s->ctx->srp_ctx.info;
	}

#define tls1_ctx_ctrl ssl3_ctx_ctrl
#define tls1_ctx_callback_ctrl ssl3_ctx_callback_ctrl

int SSL_CTX_set_srp_username(SSL_CTX *ctx,char *name)
	{
	return tls1_ctx_ctrl(ctx,SSL_CTRL_SET_TLS_EXT_SRP_USERNAME,0,name);
	}

int SSL_CTX_set_srp_password(SSL_CTX *ctx,char *password)
	{
	return tls1_ctx_ctrl(ctx,SSL_CTRL_SET_TLS_EXT_SRP_PASSWORD,0,password);
	}

int SSL_CTX_set_srp_strength(SSL_CTX *ctx, int strength)
	{
	return tls1_ctx_ctrl(ctx, SSL_CTRL_SET_TLS_EXT_SRP_STRENGTH, strength,
			     NULL);
	}

int SSL_CTX_set_srp_verify_param_callback(SSL_CTX *ctx, int (*cb)(SSL *,void *))
	{
	return tls1_ctx_callback_ctrl(ctx,SSL_CTRL_SET_SRP_VERIFY_PARAM_CB,
				      (void (*)(void))cb);
	}

int SSL_CTX_set_srp_cb_arg(SSL_CTX *ctx, void *arg)
	{
	return tls1_ctx_ctrl(ctx,SSL_CTRL_SET_SRP_ARG,0,arg);
	}

int SSL_CTX_set_srp_username_callback(SSL_CTX *ctx,
				      int (*cb)(SSL *,int *,void *))
	{
	return tls1_ctx_callback_ctrl(ctx,SSL_CTRL_SET_TLS_EXT_SRP_USERNAME_CB,
				      (void (*)(void))cb);
	}

int SSL_CTX_set_srp_client_pwd_callback(SSL_CTX *ctx, char *(*cb)(SSL *,void *))
	{
	return tls1_ctx_callback_ctrl(ctx,SSL_CTRL_SET_SRP_GIVE_CLIENT_PWD_CB,
				      (void (*)(void))cb);
	}

#endif