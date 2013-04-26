
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

#include "kadmin_locl.h"
#include "kadmin-commands.h"

static void
add_tl(kadm5_principal_ent_rec *princ, int type, krb5_data *data)
{
    krb5_tl_data *tl, **ptl;

    tl = ecalloc(1, sizeof(*tl));
    tl->tl_data_next = NULL;
    tl->tl_data_type = KRB5_TL_EXTENSION;
    tl->tl_data_length = data->length;
    tl->tl_data_contents = data->data;

    princ->n_tl_data++;
    ptl = &princ->tl_data;
    while (*ptl != NULL)
	ptl = &(*ptl)->tl_data_next;
    *ptl = tl;

    return;
}

static void
add_constrained_delegation(krb5_context contextp,
			   kadm5_principal_ent_rec *princ,
			   struct getarg_strings *strings)
{
    krb5_error_code ret;
    HDB_extension ext;
    krb5_data buf;
    size_t size = 0;

    memset(&ext, 0, sizeof(ext));
    ext.mandatory = FALSE;
    ext.data.element = choice_HDB_extension_data_allowed_to_delegate_to;

    if (strings->num_strings == 1 && strings->strings[0][0] == '\0') {
	ext.data.u.allowed_to_delegate_to.val = NULL;
	ext.data.u.allowed_to_delegate_to.len = 0;
    } else {
	krb5_principal p;
	int i;

	ext.data.u.allowed_to_delegate_to.val =
	    calloc(strings->num_strings,
		   sizeof(ext.data.u.allowed_to_delegate_to.val[0]));
	ext.data.u.allowed_to_delegate_to.len = strings->num_strings;

	for (i = 0; i < strings->num_strings; i++) {
	    ret = krb5_parse_name(contextp, strings->strings[i], &p);
	    if (ret)
		abort();
	    ret = copy_Principal(p, &ext.data.u.allowed_to_delegate_to.val[i]);
	    if (ret)
		abort();
	    krb5_free_principal(contextp, p);
	}
    }

    ASN1_MALLOC_ENCODE(HDB_extension, buf.data, buf.length,
		       &ext, &size, ret);
    free_HDB_extension(&ext);
    if (ret)
	abort();
    if (buf.length != size)
	abort();

    add_tl(princ, KRB5_TL_EXTENSION, &buf);
}

static void
add_aliases(krb5_context contextp, kadm5_principal_ent_rec *princ,
	    struct getarg_strings *strings)
{
    krb5_error_code ret;
    HDB_extension ext;
    krb5_data buf;
    krb5_principal p;
    size_t size = 0;
    int i;

    memset(&ext, 0, sizeof(ext));
    ext.mandatory = FALSE;
    ext.data.element = choice_HDB_extension_data_aliases;
    ext.data.u.aliases.case_insensitive = 0;

    if (strings->num_strings == 1 && strings->strings[0][0] == '\0') {
	ext.data.u.aliases.aliases.val = NULL;
	ext.data.u.aliases.aliases.len = 0;
    } else {
	ext.data.u.aliases.aliases.val =
	    calloc(strings->num_strings,
		   sizeof(ext.data.u.aliases.aliases.val[0]));
	ext.data.u.aliases.aliases.len = strings->num_strings;

	for (i = 0; i < strings->num_strings; i++) {
	    ret = krb5_parse_name(contextp, strings->strings[i], &p);
	    ret = copy_Principal(p, &ext.data.u.aliases.aliases.val[i]);
	    krb5_free_principal(contextp, p);
	}
    }

    ASN1_MALLOC_ENCODE(HDB_extension, buf.data, buf.length,
		       &ext, &size, ret);
    free_HDB_extension(&ext);
    if (ret)
	abort();
    if (buf.length != size)
	abort();

    add_tl(princ, KRB5_TL_EXTENSION, &buf);
}

static void
add_pkinit_acl(krb5_context contextp, kadm5_principal_ent_rec *princ,
	       struct getarg_strings *strings)
{
    krb5_error_code ret;
    HDB_extension ext;
    krb5_data buf;
    size_t size = 0;
    int i;

    memset(&ext, 0, sizeof(ext));
    ext.mandatory = FALSE;
    ext.data.element = choice_HDB_extension_data_pkinit_acl;
    ext.data.u.aliases.case_insensitive = 0;

    if (strings->num_strings == 1 && strings->strings[0][0] == '\0') {
	ext.data.u.pkinit_acl.val = NULL;
	ext.data.u.pkinit_acl.len = 0;
    } else {
	ext.data.u.pkinit_acl.val =
	    calloc(strings->num_strings,
		   sizeof(ext.data.u.pkinit_acl.val[0]));
	ext.data.u.pkinit_acl.len = strings->num_strings;

	for (i = 0; i < strings->num_strings; i++) {
	    ext.data.u.pkinit_acl.val[i].subject = estrdup(strings->strings[i]);
	}
    }

    ASN1_MALLOC_ENCODE(HDB_extension, buf.data, buf.length,
		       &ext, &size, ret);
    free_HDB_extension(&ext);
    if (ret)
	abort();
    if (buf.length != size)
	abort();

    add_tl(princ, KRB5_TL_EXTENSION, &buf);
}

static int
do_mod_entry(krb5_principal principal, void *data)
{
    krb5_error_code ret;
    kadm5_principal_ent_rec princ;
    int mask = 0;
    struct modify_options *e = data;

    memset (&princ, 0, sizeof(princ));
    ret = kadm5_get_principal(kadm_handle, principal, &princ,
			      KADM5_PRINCIPAL | KADM5_ATTRIBUTES |
			      KADM5_MAX_LIFE | KADM5_MAX_RLIFE |
			      KADM5_PRINC_EXPIRE_TIME |
			      KADM5_PW_EXPIRATION);
    if(ret)
	return ret;

    if(e->max_ticket_life_string ||
       e->max_renewable_life_string ||
       e->expiration_time_string ||
       e->pw_expiration_time_string ||
       e->attributes_string ||
       e->kvno_integer != -1 ||
       e->constrained_delegation_strings.num_strings ||
       e->alias_strings.num_strings ||
       e->pkinit_acl_strings.num_strings) {
	ret = set_entry(context, &princ, &mask,
			e->max_ticket_life_string,
			e->max_renewable_life_string,
			e->expiration_time_string,
			e->pw_expiration_time_string,
			e->attributes_string);
	if(e->kvno_integer != -1) {
	    princ.kvno = e->kvno_integer;
	    mask |= KADM5_KVNO;
	}
	if (e->constrained_delegation_strings.num_strings) {
	    add_constrained_delegation(context, &princ,
				       &e->constrained_delegation_strings);
	    mask |= KADM5_TL_DATA;
	}
	if (e->alias_strings.num_strings) {
	    add_aliases(context, &princ, &e->alias_strings);
	    mask |= KADM5_TL_DATA;
	}
	if (e->pkinit_acl_strings.num_strings) {
	    add_pkinit_acl(context, &princ, &e->pkinit_acl_strings);
	    mask |= KADM5_TL_DATA;
	}

    } else
	ret = edit_entry(&princ, &mask, NULL, 0);
    if(ret == 0) {
	ret = kadm5_modify_principal(kadm_handle, &princ, mask);
	if(ret)
	    krb5_warn(context, ret, "kadm5_modify_principal");
    }

    kadm5_free_principal_ent(kadm_handle, &princ);
    return ret;
}

int
mod_entry(struct modify_options *opt, int argc, char **argv)
{
    krb5_error_code ret = 0;
    int i;

    for(i = 0; i < argc; i++) {
	ret = foreach_principal(argv[i], do_mod_entry, "mod", opt);
	if (ret)
	    break;
    }
    return ret != 0;
}