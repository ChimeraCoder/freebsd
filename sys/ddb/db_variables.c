
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>

#include <ddb/ddb.h>
#include <ddb/db_lex.h>
#include <ddb/db_variables.h>

static int	db_find_variable(struct db_variable **varp);

static struct db_variable db_vars[] = {
	{ "radix",	&db_radix, FCN_NULL },
	{ "maxoff",	&db_maxoff, FCN_NULL },
	{ "maxwidth",	&db_max_width, FCN_NULL },
	{ "tabstops",	&db_tab_stop_width, FCN_NULL },
	{ "lines",	&db_lines_per_page, FCN_NULL },
	{ "curcpu",	NULL, db_var_curcpu },
	{ "db_cpu",	NULL, db_var_db_cpu },
#ifdef VIMAGE
	{ "curvnet",	NULL, db_var_curvnet },
	{ "db_vnet",	NULL, db_var_db_vnet },
#endif
};
static struct db_variable *db_evars =
	db_vars + sizeof(db_vars)/sizeof(db_vars[0]);

static int
db_find_variable(struct db_variable **varp)
{
	struct db_variable *vp;
	int t;

	t = db_read_token();
	if (t == tIDENT) {
		for (vp = db_vars; vp < db_evars; vp++) {
			if (!strcmp(db_tok_string, vp->name)) {
				*varp = vp;
				return (1);
			}
		}
		for (vp = db_regs; vp < db_eregs; vp++) {
			if (!strcmp(db_tok_string, vp->name)) {
				*varp = vp;
				return (1);
			}
		}
	}
	db_error("Unknown variable\n");
	return (0);
}

int
db_get_variable(db_expr_t *valuep)
{
	struct db_variable *vp;

	if (!db_find_variable(&vp))
		return (0);

	return (db_read_variable(vp, valuep));
}

int
db_set_variable(db_expr_t value)
{
	struct db_variable *vp;

	if (!db_find_variable(&vp))
		return (0);

	return (db_write_variable(vp, value));
}

int
db_read_variable(struct db_variable *vp, db_expr_t *valuep)
{
	db_varfcn_t *func = vp->fcn;

	if (func == FCN_NULL) {
		*valuep = *(vp->valuep);
		return (1);
	}
	return ((*func)(vp, valuep, DB_VAR_GET));
}

int
db_write_variable(struct db_variable *vp, db_expr_t value)
{
	db_varfcn_t *func = vp->fcn;

	if (func == FCN_NULL) {
		*(vp->valuep) = value;
		return (1);
	}
	return ((*func)(vp, &value, DB_VAR_SET));
}

void
db_set_cmd(db_expr_t dummy1, boolean_t dummy2, db_expr_t dummy3, char *dummy4)
{
	struct db_variable *vp;
	db_expr_t value;
	int t;

	t = db_read_token();
	if (t != tDOLLAR) {
		db_error("Unknown variable\n");
		return;
	}
	if (!db_find_variable(&vp)) {
		db_error("Unknown variable\n");
		return;
	}

	t = db_read_token();
	if (t != tEQ)
		db_unread_token(t);

	if (!db_expression(&value)) {
		db_error("No value\n");
		return;
	}
	if (db_read_token() != tEOL)
		db_error("?\n");

	db_write_variable(vp, value);
}