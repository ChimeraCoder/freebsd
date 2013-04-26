
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

#include <ddb/ddb.h>
#include <ddb/db_lex.h>
#include <ddb/db_access.h>
#include <ddb/db_command.h>

static boolean_t	db_add_expr(db_expr_t *valuep);
static boolean_t	db_mult_expr(db_expr_t *valuep);
static boolean_t	db_shift_expr(db_expr_t *valuep);
static boolean_t	db_term(db_expr_t *valuep);
static boolean_t	db_unary(db_expr_t *valuep);

static boolean_t
db_term(db_expr_t *valuep)
{
	int	t;

	t = db_read_token();
	if (t == tIDENT) {
	    if (!db_value_of_name(db_tok_string, valuep) &&
		!db_value_of_name_pcpu(db_tok_string, valuep) &&
		!db_value_of_name_vnet(db_tok_string, valuep)) {
		db_error("Symbol not found\n");
		/*NOTREACHED*/
	    }
	    return (TRUE);
	}
	if (t == tNUMBER) {
	    *valuep = (db_expr_t)db_tok_number;
	    return (TRUE);
	}
	if (t == tDOT) {
	    *valuep = (db_expr_t)db_dot;
	    return (TRUE);
	}
	if (t == tDOTDOT) {
	    *valuep = (db_expr_t)db_prev;
	    return (TRUE);
	}
	if (t == tPLUS) {
	    *valuep = (db_expr_t) db_next;
	    return (TRUE);
	}
	if (t == tDITTO) {
	    *valuep = (db_expr_t)db_last_addr;
	    return (TRUE);
	}
	if (t == tDOLLAR) {
	    if (!db_get_variable(valuep))
		return (FALSE);
	    return (TRUE);
	}
	if (t == tLPAREN) {
	    if (!db_expression(valuep)) {
		db_error("Syntax error\n");
		/*NOTREACHED*/
	    }
	    t = db_read_token();
	    if (t != tRPAREN) {
		db_error("Syntax error\n");
		/*NOTREACHED*/
	    }
	    return (TRUE);
	}
	db_unread_token(t);
	return (FALSE);
}

static boolean_t
db_unary(db_expr_t *valuep)
{
	int	t;

	t = db_read_token();
	if (t == tMINUS) {
	    if (!db_unary(valuep)) {
		db_error("Syntax error\n");
		/*NOTREACHED*/
	    }
	    *valuep = -*valuep;
	    return (TRUE);
	}
	if (t == tSTAR) {
	    /* indirection */
	    if (!db_unary(valuep)) {
		db_error("Syntax error\n");
		/*NOTREACHED*/
	    }
	    *valuep = db_get_value((db_addr_t)*valuep, sizeof(void *), FALSE);
	    return (TRUE);
	}
	db_unread_token(t);
	return (db_term(valuep));
}

static boolean_t
db_mult_expr(db_expr_t *valuep)
{
	db_expr_t	lhs, rhs;
	int		t;

	if (!db_unary(&lhs))
	    return (FALSE);

	t = db_read_token();
	while (t == tSTAR || t == tSLASH || t == tPCT || t == tHASH) {
	    if (!db_term(&rhs)) {
		db_error("Syntax error\n");
		/*NOTREACHED*/
	    }
	    if (t == tSTAR)
		lhs *= rhs;
	    else {
		if (rhs == 0) {
		    db_error("Divide by 0\n");
		    /*NOTREACHED*/
		}
		if (t == tSLASH)
		    lhs /= rhs;
		else if (t == tPCT)
		    lhs %= rhs;
		else
		    lhs = ((lhs+rhs-1)/rhs)*rhs;
	    }
	    t = db_read_token();
	}
	db_unread_token(t);
	*valuep = lhs;
	return (TRUE);
}

static boolean_t
db_add_expr(db_expr_t *valuep)
{
	db_expr_t	lhs, rhs;
	int		t;

	if (!db_mult_expr(&lhs))
	    return (FALSE);

	t = db_read_token();
	while (t == tPLUS || t == tMINUS) {
	    if (!db_mult_expr(&rhs)) {
		db_error("Syntax error\n");
		/*NOTREACHED*/
	    }
	    if (t == tPLUS)
		lhs += rhs;
	    else
		lhs -= rhs;
	    t = db_read_token();
	}
	db_unread_token(t);
	*valuep = lhs;
	return (TRUE);
}

static boolean_t
db_shift_expr(db_expr_t *valuep)
{
	db_expr_t	lhs, rhs;
	int		t;

	if (!db_add_expr(&lhs))
	    return (FALSE);

	t = db_read_token();
	while (t == tSHIFT_L || t == tSHIFT_R) {
	    if (!db_add_expr(&rhs)) {
		db_error("Syntax error\n");
		/*NOTREACHED*/
	    }
	    if (rhs < 0) {
		db_error("Negative shift amount\n");
		/*NOTREACHED*/
	    }
	    if (t == tSHIFT_L)
		lhs <<= rhs;
	    else {
		/* Shift right is unsigned */
		lhs = (unsigned) lhs >> rhs;
	    }
	    t = db_read_token();
	}
	db_unread_token(t);
	*valuep = lhs;
	return (TRUE);
}

int
db_expression(db_expr_t *valuep)
{
	return (db_shift_expr(valuep));
}