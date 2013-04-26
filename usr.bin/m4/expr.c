
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

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include "mdef.h"
#include "extern.h"

int32_t end_result;
static const char *copy_toeval;
int yyerror(const char *msg);

extern void yy_scan_string(const char *);
extern int yyparse(void);

int
yyerror(const char *msg)
{
	fprintf(stderr, "m4: %s in expr %s\n", msg, copy_toeval);
	return(0);
}

int
expr(const char *toeval)
{
	copy_toeval = toeval;
	yy_scan_string(toeval);
	yyparse();
	return end_result;
}