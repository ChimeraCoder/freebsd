
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

#include "hdb_locl.h"

struct hdb_dbinfo {
    char *label;
    char *realm;
    char *dbname;
    char *mkey_file;
    char *acl_file;
    char *log_file;
    const krb5_config_binding *binding;
    struct hdb_dbinfo *next;
};

static int
get_dbinfo(krb5_context context,
	   const krb5_config_binding *db_binding,
	   const char *label,
	   struct hdb_dbinfo **db)
{
    struct hdb_dbinfo *di;
    const char *p;

    *db = NULL;

    p = krb5_config_get_string(context, db_binding, "dbname", NULL);
    if(p == NULL)
	return 0;

    di = calloc(1, sizeof(*di));
    if (di == NULL) {
	krb5_set_error_message(context, ENOMEM, "malloc: out of memory");
	return ENOMEM;
    }
    di->label = strdup(label);
    di->dbname = strdup(p);

    p = krb5_config_get_string(context, db_binding, "realm", NULL);
    if(p)
	di->realm = strdup(p);
    p = krb5_config_get_string(context, db_binding, "mkey_file", NULL);
    if(p)
	di->mkey_file = strdup(p);
    p = krb5_config_get_string(context, db_binding, "acl_file", NULL);
    if(p)
	di->acl_file = strdup(p);
    p = krb5_config_get_string(context, db_binding, "log_file", NULL);
    if(p)
	di->log_file = strdup(p);

    di->binding = db_binding;

    *db = di;
    return 0;
}


int
hdb_get_dbinfo(krb5_context context, struct hdb_dbinfo **dbp)
{
    const krb5_config_binding *db_binding;
    struct hdb_dbinfo *di, **dt, *databases;
    const char *default_dbname = HDB_DEFAULT_DB;
    const char *default_mkey = HDB_DB_DIR "/m-key";
    const char *default_acl = HDB_DB_DIR "/kadmind.acl";
    const char *p;
    int ret;

    *dbp = NULL;
    dt = NULL;
    databases = NULL;

    db_binding = krb5_config_get_list(context, NULL,
				      "kdc",
				      "database",
				      NULL);
    if (db_binding) {

	ret = get_dbinfo(context, db_binding, "default", &di);
	if (ret == 0 && di) {
	    databases = di;
	    dt = &di->next;
	}

	for ( ; db_binding != NULL; db_binding = db_binding->next) {

	    if (db_binding->type != krb5_config_list)
		continue;

	    ret = get_dbinfo(context, db_binding->u.list,
			     db_binding->name, &di);
	    if (ret)
		krb5_err(context, 1, ret, "failed getting realm");

	    if (di == NULL)
		continue;

	    if (dt)
		*dt = di;
	    else
		databases = di;
	    dt = &di->next;

	}
    }

    if(databases == NULL) {
	/* if there are none specified, create one and use defaults */
	di = calloc(1, sizeof(*di));
	databases = di;
	di->label = strdup("default");
    }

    for(di = databases; di; di = di->next) {
	if(di->dbname == NULL) {
	    di->dbname = strdup(default_dbname);
	    if (di->mkey_file == NULL)
		di->mkey_file = strdup(default_mkey);
	}
	if(di->mkey_file == NULL) {
	    p = strrchr(di->dbname, '.');
	    if(p == NULL || strchr(p, '/') != NULL)
		/* final pathname component does not contain a . */
		asprintf(&di->mkey_file, "%s.mkey", di->dbname);
	    else
		/* the filename is something.else, replace .else with
                   .mkey */
		asprintf(&di->mkey_file, "%.*s.mkey",
			 (int)(p - di->dbname), di->dbname);
	}
	if(di->acl_file == NULL)
	    di->acl_file = strdup(default_acl);
    }
    *dbp = databases;
    return 0;
}


struct hdb_dbinfo *
hdb_dbinfo_get_next(struct hdb_dbinfo *dbp, struct hdb_dbinfo *dbprevp)
{
    if (dbprevp == NULL)
	return dbp;
    else
	return dbprevp->next;
}

const char *
hdb_dbinfo_get_label(krb5_context context, struct hdb_dbinfo *dbp)
{
    return dbp->label;
}

const char *
hdb_dbinfo_get_realm(krb5_context context, struct hdb_dbinfo *dbp)
{
    return dbp->realm;
}

const char *
hdb_dbinfo_get_dbname(krb5_context context, struct hdb_dbinfo *dbp)
{
    return dbp->dbname;
}

const char *
hdb_dbinfo_get_mkey_file(krb5_context context, struct hdb_dbinfo *dbp)
{
    return dbp->mkey_file;
}

const char *
hdb_dbinfo_get_acl_file(krb5_context context, struct hdb_dbinfo *dbp)
{
    return dbp->acl_file;
}

const char *
hdb_dbinfo_get_log_file(krb5_context context, struct hdb_dbinfo *dbp)
{
    return dbp->log_file;
}

const krb5_config_binding *
hdb_dbinfo_get_binding(krb5_context context, struct hdb_dbinfo *dbp)
{
    return dbp->binding;
}

void
hdb_free_dbinfo(krb5_context context, struct hdb_dbinfo **dbp)
{
    struct hdb_dbinfo *di, *ndi;

    for(di = *dbp; di != NULL; di = ndi) {
	ndi = di->next;
	free (di->label);
	free (di->realm);
	free (di->dbname);
	free (di->mkey_file);
	free (di->acl_file);
	free (di->log_file);
	free(di);
    }
    *dbp = NULL;
}

/**
 * Return the directory where the hdb database resides.
 *
 * @param context Kerberos 5 context.
 *
 * @return string pointing to directory.
 */

const char *
hdb_db_dir(krb5_context context)
{
    return HDB_DB_DIR;
}

/**
 * Return the default hdb database resides.
 *
 * @param context Kerberos 5 context.
 *
 * @return string pointing to directory.
 */

const char *
hdb_default_db(krb5_context context)
{
    return HDB_DEFAULT_DB;
}