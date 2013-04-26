
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

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/lhash.h>
#include <openssl/objects.h>
#include <openssl/buffer.h>

ASN1_OBJECT *OBJ_dup(const ASN1_OBJECT *o)
	{
	ASN1_OBJECT *r;
	int i;
	char *ln=NULL,*sn=NULL;
	unsigned char *data=NULL;

	if (o == NULL) return(NULL);
	if (!(o->flags & ASN1_OBJECT_FLAG_DYNAMIC))
		return((ASN1_OBJECT *)o); /* XXX: ugh! Why? What kind of
					     duplication is this??? */

	r=ASN1_OBJECT_new();
	if (r == NULL)
		{
		OBJerr(OBJ_F_OBJ_DUP,ERR_R_ASN1_LIB);
		return(NULL);
		}
	data=OPENSSL_malloc(o->length);
	if (data == NULL)
		goto err;
	if (o->data != NULL)
		memcpy(data,o->data,o->length);
	/* once data attached to object it remains const */
	r->data = data;
	r->length=o->length;
	r->nid=o->nid;
	r->ln=r->sn=NULL;
	if (o->ln != NULL)
		{
		i=strlen(o->ln)+1;
		ln=OPENSSL_malloc(i);
		if (ln == NULL) goto err;
		memcpy(ln,o->ln,i);
		r->ln=ln;
		}

	if (o->sn != NULL)
		{
		i=strlen(o->sn)+1;
		sn=OPENSSL_malloc(i);
		if (sn == NULL) goto err;
		memcpy(sn,o->sn,i);
		r->sn=sn;
		}
	r->flags=o->flags|(ASN1_OBJECT_FLAG_DYNAMIC|
		ASN1_OBJECT_FLAG_DYNAMIC_STRINGS|ASN1_OBJECT_FLAG_DYNAMIC_DATA);
	return(r);
err:
	OBJerr(OBJ_F_OBJ_DUP,ERR_R_MALLOC_FAILURE);
	if (ln != NULL)		OPENSSL_free(ln);
	if (sn != NULL)		OPENSSL_free(sn);
	if (data != NULL)	OPENSSL_free(data);
	if (r != NULL)		OPENSSL_free(r);
	return(NULL);
	}

int OBJ_cmp(const ASN1_OBJECT *a, const ASN1_OBJECT *b)
	{
	int ret;

	ret=(a->length-b->length);
	if (ret) return(ret);
	return(memcmp(a->data,b->data,a->length));
	}