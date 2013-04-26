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

# keep the editing independent of locale:
if test "${LANGUAGE+set}"    = set; then LANGUAGE=C;    export LANGUAGE;    fi
if test "${LANG+set}"        = set; then LANG=C;        export LANG;        fi
if test "${LC_ALL+set}"      = set; then LC_ALL=C;      export LC_ALL;      fi
if test "${LC_MESSAGES+set}" = set; then LC_MESSAGES=C; export LC_MESSAGES; fi
if test "${LC_CTYPE+set}"    = set; then LC_CTYPE=C;    export LC_CTYPE;    fi
if test "${LC_COLLATE+set}"  = set; then LC_COLLATE=C;  export LC_COLLATE;  fi

preprocessor="$1 -DNCURSES_INTERNALS -I../include"
AWK="$2"
USE="$3"

PID=$$
ED1=sed1_${PID}.sed
ED2=sed2_${PID}.sed
ED3=sed3_${PID}.sed
ED4=sed4_${PID}.sed
AW1=awk1_${PID}.awk
AW2=awk2_${PID}.awk
TMP=gen__${PID}.c
trap "rm -f $ED1 $ED2 $ED3 $ED4 $AW1 $AW2 $TMP" 0 1 2 5 15

ALL=$USE
if test "$USE" = implemented ; then
	CALL="call_"
	cat >$ED1 <<EOF1
/^extern.*implemented/{
	h
	s/^.*implemented:\([^ 	*]*\).*/P_POUNDCif_USE_\1_SUPPORT/p
	g
	s/^extern \([^;]*\);.*/\1/p
	g
	s/^.*implemented:\([^ 	*]*\).*/P_POUNDCendif/p
}
/^extern.*generated/{
	h
	s/^.*generated:\([^ 	*]*\).*/P_POUNDCif_USE_\1_SUPPORT/p
	g
	s/^extern \([^;]*\);.*/\1/p
	g
	s/^.*generated:\([^ 	*]*\).*/P_POUNDCendif/p
}
EOF1
else
	CALL=""
	cat >$ED1 <<EOF1
/^extern.*${ALL}/{
	h
	s/^.*${ALL}:\([^ 	*]*\).*/P_POUNDCif_USE_\1_SUPPORT/p
	g
	s/^extern \([^;]*\);.*/\1/p
	g
	s/^.*${ALL}:\([^ 	*]*\).*/P_POUNDCendif/p
}
EOF1
fi

cat >$ED2 <<EOF2
/^P_/b nc
/(void)/b nc
	s/,/ a1% /
	s/,/ a2% /
	s/,/ a3% /
	s/,/ a4% /
	s/,/ a5% /
	s/,/ a6% /
	s/,/ a7% /
	s/,/ a8% /
	s/,/ a9% /
	s/,/ a10% /
	s/,/ a11% /
	s/,/ a12% /
	s/,/ a13% /
	s/,/ a14% /
	s/,/ a15% /
	s/*/ * /g
	s/%/ , /g
	s/)/ z)/
	s/\.\.\. z)/...)/
:nc
	s/(/ ( /
	s/)/ )/
EOF2

cat >$ED3 <<EOF3
/^P_/{
	s/^P_POUNDCif_/#if /
	s/^P_POUNDCendif/#endif/
	s/^P_//
	b done
}
	s/		*/ /g
	s/  */ /g
	s/ ,/,/g
	s/( /(/g
	s/ )/)/g
	s/ gen_/ /
	s/^M_/#undef /
	s/^[ 	]*@[ 	]*@[ 	]*/	/
:done
EOF3

if test "$USE" = generated ; then
cat >$ED4 <<EOF
	s/^\(.*\) \(.*\) (\(.*\))\$/NCURSES_EXPORT(\1) \2 (\3)/
EOF
else
cat >$ED4 <<EOF
/^\(.*\) \(.*\) (\(.*\))\$/ {
	h
	s/^\(.*\) \(.*\) (\(.*\))\$/extern \1 call_\2 (\3);/
	p
	g
	s/^\(.*\) \(.*\) (\(.*\))\$/\1 call_\2 (\3)/
	}
EOF
fi

cat >$AW1 <<\EOF1
BEGIN	{
		skip=0;
	}
/^P_POUNDCif/ {
		print "\n"
		print $0
		skip=0;
}
/^P_POUNDCendif/ {
		print $0
		skip=1;
}
$0 !~ /^P_/ {
	if (skip)
		print "\n"
	skip=1;

	first=$1
	for (i = 1; i <= NF; i++) {
		if ( $i != "NCURSES_CONST" ) {
			first = i;
			break;
		}
	}
	second = first + 1;
	if ( $first == "chtype" ) {
		returnType = "Char";
	} else if ( $first == "SCREEN" ) {
		returnType = "SP";
	} else if ( $first == "WINDOW" ) {
		returnType = "Win";
	} else if ( $first == "attr_t" || $second == "attrset" || $second == "standout" || $second == "standend" || $second == "wattrset" || $second == "wstandout" || $second == "wstandend" ) {
		returnType = "Attr";
	} else if ( $first == "bool" || $first == "NCURSES_BOOL" ) {
		returnType = "Bool";
	} else if ( $second == "*" ) {
		returnType = "Ptr";
	} else {
		returnType = "Code";
	}
	myfunc = second;
	for (i = second; i <= NF; i++) {
		if ($i != "*") {
			myfunc = i;
			break;
		}
	}
	if (using == "generated") {
		print "M_" $myfunc
	}
	print $0;
	print "{";
	argcount = 1;
	check = NF - 1;
	if ($check == "void")
		argcount = 0;
	if (argcount != 0) {
		for (i = 1; i <= NF; i++)
			if ($i == ",")
				argcount++;
	}

	# suppress trace-code for functions that we cannot do properly here,
	# since they return data.
	dotrace = 1;
	if ($myfunc ~ /innstr/)
		dotrace = 0;
	if ($myfunc ~ /innwstr/)
		dotrace = 0;

	# workaround functions that we do not parse properly
	if ($myfunc ~ /ripoffline/) {
		dotrace = 0;
		argcount = 2;
	}
	if ($myfunc ~ /wunctrl/) {
		dotrace = 0;
	}

	call = "@@T((T_CALLED(\""
	args = ""
	comma = ""
	num = 0;
	pointer = 0;
	va_list = 0;
	varargs = 0;
	argtype = ""
	for (i = myfunc; i <= NF; i++) {
		ch = $i;
		if ( ch == "*" )
			pointer = 1;
		else if ( ch == "va_list" )
			va_list = 1;
		else if ( ch == "..." )
			varargs = 1;
		else if ( ch == "char" )
			argtype = "char";
		else if ( ch == "int" )
			argtype = "int";
		else if ( ch == "short" )
			argtype = "short";
		else if ( ch == "chtype" )
			argtype = "chtype";
		else if ( ch == "attr_t" || ch == "NCURSES_ATTR_T" )
			argtype = "attr";

		if ( ch == "," || ch == ")" ) {
			if (va_list) {
				call = call "%s"
			} else if (varargs) {
				call = call "%s"
			} else if (pointer) {
				if ( argtype == "char" ) {
					call = call "%s"
					comma = comma "_nc_visbuf2(" num ","
					pointer = 0;
				} else
					call = call "%p"
			} else if (argcount != 0) {
				if ( argtype == "int" || argtype == "short" ) {
					call = call "%d"
					argtype = ""
				} else if ( argtype != "" ) {
					call = call "%s"
					comma = comma "_trace" argtype "2(" num ","
				} else {
					call = call "%#lx"
					comma = comma "(long)"
				}
			}
			if (ch == ",") {
				args = args comma "a" ++num;
			} else if ( argcount != 0 ) {
				if ( va_list ) {
					args = args comma "\"va_list\""
				} else if ( varargs ) {
					args = args comma "\"...\""
				} else {
					args = args comma "z"
				}
			}
			call = call ch
			if (pointer == 0 && argcount != 0 && argtype != "" )
				args = args ")"
			if (args != "")
				comma = ", "
			pointer = 0;
			argtype = ""
		}
		if ( i == 2 || ch == "(" )
			call = call ch
	}
	call = call "\")"
	if (args != "")
		call = call ", " args
	call = call ")); "

	if (dotrace)
		printf "%s", call

	if (match($0, "^void"))
		call = ""
	else if (dotrace)
		call = sprintf("return%s( ", returnType);
	else
		call = "@@return ";

	call = call $myfunc "(";
	for (i = 1; i < argcount; i++) {
		if (i != 1)
			call = call ", ";
		call = call "a" i;
	}
	if ( argcount != 0 && $check != "..." ) {
		if (argcount != 1)
			call = call ", ";
		call = call "z";
	}
	if (!match($0, "^void"))
		call = call ") ";
	if (dotrace)
		call = call ")";
	print call ";"

	if (match($0, "^void"))
		print "@@returnVoid;"
	print "}";
}
EOF1

cat >$AW2 <<EOF1
BEGIN		{
		print "/*"
		print " * DO NOT EDIT THIS FILE BY HAND!"
		printf " * It is generated by $0 %s.\n", "$USE"
		if ( "$USE" == "generated" ) {
			print " *"
			print " * This is a file of trivial functions generated from macro"
			print " * definitions in curses.h to satisfy the XSI Curses requirement"
			print " * that every macro also exist as a callable function."
			print " *"
			print " * It will never be linked unless you call one of the entry"
			print " * points with its normal macro definition disabled.  In that"
			print " * case, if you have no shared libraries, it will indirectly"
			print " * pull most of the rest of the library into your link image."
		}
		print " */"
		print "#define NCURSES_ATTR_T int"
		print "#include <curses.priv.h>"
		print ""
		}
/^DECLARATIONS/	{start = 1; next;}
		{if (start) print \$0;}
END		{
		if ( "$USE" != "generated" ) {
			print "int main(void) { return 0; }"
		}
		}
EOF1

cat >$TMP <<EOF
#include <ncurses_cfg.h>
#undef NCURSES_NOMACROS
#include <curses.h>

DECLARATIONS

EOF

sed -n -f $ED1 \
| sed -e 's/NCURSES_EXPORT(\(.*\)) \(.*\) (\(.*\))/\1 \2(\3)/' \
| sed -f $ED2 \
| $AWK -f $AW1 using=$USE \
| sed \
	-e 's/ [ ]*$//g' \
	-e 's/^\([a-zA-Z_][a-zA-Z_]*[ *]*\)/\1 gen_/' \
	-e 's/gen_$//' \
	-e 's/  / /g' >>$TMP

$preprocessor $TMP 2>/dev/null \
| sed \
	-e 's/  / /g' \
	-e 's/^ //' \
	-e 's/_Bool/NCURSES_BOOL/g' \
| $AWK -f $AW2 \
| sed -f $ED3 \
| sed \
	-e 's/^.*T_CALLED.*returnCode( \([a-z].*) \));/	return \1;/' \
	-e 's/^.*T_CALLED.*returnCode( \((wmove.*) \));/	return \1;/' \
	-e 's/gen_//' \
	-e 's/^[ 	]*#/#/' \
	-e '/#ident/d' \
	-e '/#line/d' \
| sed -f $ED4