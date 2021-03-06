
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

#include <config.h>

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "roken.h"
#include "parse_units.h"

/*
 * Parse string in `s' according to `units' and return value.
 * def_unit defines the default unit.
 */

static int
parse_something (const char *s, const struct units *units,
		 const char *def_unit,
		 int (*func)(int res, int val, unsigned mult),
		 int init,
		 int accept_no_val_p)
{
    const char *p;
    int res = init;
    unsigned def_mult = 1;

    if (def_unit != NULL) {
	const struct units *u;

	for (u = units; u->name; ++u) {
	    if (strcasecmp (u->name, def_unit) == 0) {
		def_mult = u->mult;
		break;
	    }
	}
	if (u->name == NULL)
	    return -1;
    }

    p = s;
    while (*p) {
	int val;
	char *next;
	const struct units *u, *partial_unit;
	size_t u_len;
	unsigned partial;
	int no_val_p = 0;

	while(isspace((unsigned char)*p) || *p == ',')
	    ++p;

	val = strtol(p, &next, 0);
	if (p == next) {
	    val = 0;
	    if(!accept_no_val_p)
		return -1;
	    no_val_p = 1;
	}
	p = next;
	while (isspace((unsigned char)*p))
	    ++p;
	if (*p == '\0') {
	    res = (*func)(res, val, def_mult);
	    if (res < 0)
		return res;
	    break;
	} else if (*p == '+') {
	    ++p;
	    val = 1;
	} else if (*p == '-') {
	    ++p;
	    val = -1;
	}
	if (no_val_p && val == 0)
	    val = 1;
	u_len = strcspn (p, ", \t");
	partial = 0;
	partial_unit = NULL;
	if (u_len > 1 && p[u_len - 1] == 's')
	    --u_len;
	for (u = units; u->name; ++u) {
	    if (strncasecmp (p, u->name, u_len) == 0) {
		if (u_len == strlen (u->name)) {
		    p += u_len;
		    res = (*func)(res, val, u->mult);
		    if (res < 0)
			return res;
		    break;
		} else {
		    ++partial;
		    partial_unit = u;
		}
	    }
	}
	if (u->name == NULL) {
	    if (partial == 1) {
		p += u_len;
		res = (*func)(res, val, partial_unit->mult);
		if (res < 0)
		    return res;
	    } else {
		return -1;
	    }
	}
	if (*p == 's')
	    ++p;
    }
    return res;
}

/*
 * The string consists of a sequence of `n unit'
 */

static int
acc_units(int res, int val, unsigned mult)
{
    return res + val * mult;
}

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
parse_units (const char *s, const struct units *units,
	     const char *def_unit)
{
    return parse_something (s, units, def_unit, acc_units, 0, 0);
}

/*
 * The string consists of a sequence of `[+-]flag'.  `orig' consists
 * the original set of flags, those are then modified and returned as
 * the function value.
 */

static int
acc_flags(int res, int val, unsigned mult)
{
    if(val == 1)
	return res | mult;
    else if(val == -1)
	return res & ~mult;
    else if (val == 0)
	return mult;
    else
	return -1;
}

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
parse_flags (const char *s, const struct units *units,
	     int orig)
{
    return parse_something (s, units, NULL, acc_flags, orig, 1);
}

/*
 * Return a string representation according to `units' of `num' in `s'
 * with maximum length `len'.  The actual length is the function value.
 */

static int
unparse_something (int num, const struct units *units, char *s, size_t len,
		   int (*print) (char *, size_t, int, const char *, int),
		   int (*update) (int, unsigned),
		   const char *zero_string)
{
    const struct units *u;
    int ret = 0, tmp;

    if (num == 0)
	return snprintf (s, len, "%s", zero_string);

    for (u = units; num > 0 && u->name; ++u) {
	int divisor;

	divisor = num / u->mult;
	if (divisor) {
	    num = (*update) (num, u->mult);
	    tmp = (*print) (s, len, divisor, u->name, num);
	    if (tmp < 0)
		return tmp;
	    if (tmp > (int) len) {
		len = 0;
		s = NULL;
	    } else {
		len -= tmp;
		s += tmp;
	    }
	    ret += tmp;
	}
    }
    return ret;
}

static int
print_unit (char *s, size_t len, int divisor, const char *name, int rem)
{
    return snprintf (s, len, "%u %s%s%s",
		     divisor, name,
		     divisor == 1 ? "" : "s",
		     rem > 0 ? " " : "");
}

static int
update_unit (int in, unsigned mult)
{
    return in % mult;
}

static int
update_unit_approx (int in, unsigned mult)
{
    if (in / mult > 0)
	return 0;
    else
	return update_unit (in, mult);
}

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
unparse_units (int num, const struct units *units, char *s, size_t len)
{
    return unparse_something (num, units, s, len,
			      print_unit,
			      update_unit,
			      "0");
}

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
unparse_units_approx (int num, const struct units *units, char *s, size_t len)
{
    return unparse_something (num, units, s, len,
			      print_unit,
			      update_unit_approx,
			      "0");
}

ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
print_units_table (const struct units *units, FILE *f)
{
    const struct units *u, *u2;
    size_t max_sz = 0;

    for (u = units; u->name; ++u) {
	max_sz = max(max_sz, strlen(u->name));
    }

    for (u = units; u->name;) {
	char buf[1024];
	const struct units *next;

	for (next = u + 1; next->name && next->mult == u->mult; ++next)
	    ;

	if (next->name) {
	    for (u2 = next;
		 u2->name && u->mult % u2->mult != 0;
		 ++u2)
		;
	    if (u2->name == NULL)
		--u2;
	    unparse_units (u->mult, u2, buf, sizeof(buf));
	    fprintf (f, "1 %*s = %s\n", (int)max_sz, u->name, buf);
	} else {
	    fprintf (f, "1 %s\n", u->name);
	}
	u = next;
    }
}

static int
print_flag (char *s, size_t len, int divisor, const char *name, int rem)
{
    return snprintf (s, len, "%s%s", name, rem > 0 ? ", " : "");
}

static int
update_flag (int in, unsigned mult)
{
    return in - mult;
}

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
unparse_flags (int num, const struct units *units, char *s, size_t len)
{
    return unparse_something (num, units, s, len,
			      print_flag,
			      update_flag,
			      "");
}

ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
print_flags_table (const struct units *units, FILE *f)
{
    const struct units *u;

    for(u = units; u->name; ++u)
	fprintf(f, "%s%s", u->name, (u+1)->name ? ", " : "\n");
}