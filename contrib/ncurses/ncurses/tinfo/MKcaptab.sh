#!/bin/sh
# You may redistribute this program and/or modify it under the terms of
# the GNU General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
AWK=${1-awk}
OPT1=${2-0}
OPT2=${3-tinfo/MKcaptab.awk}
DATA=${4-../include/Caps}

cat <<'EOF'
/*
 *	comp_captab.c -- The names of the capabilities indexed via a hash
 *		         table for the compiler.
 *
 */

#include <curses.priv.h>
#include <tic.h>
#include <hashsize.h>

EOF

./make_hash 1 info $OPT1 <$DATA
./make_hash 3 cap  $OPT1 <$DATA

$AWK -f $OPT2 bigstrings=$OPT1 tablename=capalias <$DATA 

$AWK -f $OPT2 bigstrings=$OPT1 tablename=infoalias <$DATA

cat <<EOF

#if $OPT1
static void
next_string(const char *strings, unsigned *offset)
{
	*offset += strlen(strings + *offset) + 1;
}

static const struct name_table_entry *
_nc_build_names(struct name_table_entry **actual,
		const name_table_data *source,
		const char *strings)
{
	if (*actual == 0) {
		*actual = typeCalloc(struct name_table_entry, CAPTABSIZE);
		if (*actual != 0) {
			unsigned n;
			unsigned len = 0;
			for (n = 0; n < CAPTABSIZE; ++n) {
				(*actual)[n].nte_name = strings + len;
				(*actual)[n].nte_type = source[n].nte_type;
				(*actual)[n].nte_index = source[n].nte_index;
				(*actual)[n].nte_link = source[n].nte_link;
				next_string(strings, &len);
			}
		}
	}
	return *actual;
}

#define add_alias(field) \\
	if (source[n].field >= 0) { \\
		(*actual)[n].field = strings + source[n].field; \\
	}

static const struct alias *
_nc_build_alias(struct alias **actual,
		const alias_table_data *source,
		const char *strings,
		unsigned tablesize)
{
	if (*actual == 0) {
		*actual = typeCalloc(struct alias, tablesize + 1);
		if (*actual != 0) {
			unsigned n;
			for (n = 0; n < tablesize; ++n) {
				add_alias(from);
				add_alias(to);
				add_alias(source);
			}
		}
	}
	return *actual;
}

#define build_names(root) _nc_build_names(&_nc_##root##_table, \\
					  root##_names_data, \\
					  root##_names_text)
#define build_alias(root) _nc_build_alias(&_nc_##root##alias_table, \\
					  root##alias_data, \\
					  root##alias_text, \\
					  SIZEOF(root##alias_data))
#else
#define build_names(root) _nc_ ## root ## _table
#define build_alias(root) _nc_ ## root ## alias_table
#endif

NCURSES_EXPORT(const struct name_table_entry *) _nc_get_table (bool termcap)
{
	return termcap ? build_names(cap) : build_names(info) ;
}

NCURSES_EXPORT(const short *) _nc_get_hash_table (bool termcap)
{
	return termcap ? _nc_cap_hash_table: _nc_info_hash_table ;
}

NCURSES_EXPORT(const struct alias *) _nc_get_alias_table (bool termcap)
{
	return termcap ? build_alias(cap) : build_alias(info) ;
}

#if NO_LEAKS
NCURSES_EXPORT(void) _nc_comp_captab_leaks(void)
{
#if $OPT1
	FreeIfNeeded(_nc_cap_table);
	FreeIfNeeded(_nc_info_table);
	FreeIfNeeded(_nc_capalias_table);
	FreeIfNeeded(_nc_infoalias_table);
#endif
}
#endif /* NO_LEAKS */
EOF