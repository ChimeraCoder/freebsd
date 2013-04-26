
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
		/* ZZZZZZZZZZZZZZZZ */		return(NULL);
		}
	memset(ret,0,sizeof(COMP_CTX));
	ret->meth=meth;
	if ((ret->meth->init != NULL) && !ret->meth->init(ret))
		{
		OPENSSL_free(ret);
		ret=NULL;
		}
	return(ret);
	}

void COMP_CTX_free(COMP_CTX *ctx)
	{
	if(ctx == NULL)
	    return;

	if (ctx->meth->finish != NULL)
		ctx->meth->finish(ctx);

	OPENSSL_free(ctx);
	}

int COMP_compress_block(COMP_CTX *ctx, unsigned char *out, int olen,
	     unsigned char *in, int ilen)
	{
	int ret;
	if (ctx->meth->compress == NULL)
		{
		/* ZZZZZZZZZZZZZZZZZ */
		return(-1);
		}
	ret=ctx->meth->compress(ctx,out,olen,in,ilen);
	if (ret > 0)
		{
		ctx->compress_in+=ilen;
		ctx->compress_out+=ret;
		}
	return(ret);
	}

int COMP_expand_block(COMP_CTX *ctx, unsigned char *out, int olen,
	     unsigned char *in, int ilen)
	{
	int ret;

	if (ctx->meth->expand == NULL)
		{
		/* ZZZZZZZZZZZZZZZZZ */
		return(-1);
		}
	ret=ctx->meth->expand(ctx,out,olen,in,ilen);
	if (ret > 0)
		{
		ctx->expand_in+=ilen;
		ctx->expand_out+=ret;
		}
	return(ret);
	}