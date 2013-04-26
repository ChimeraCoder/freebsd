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
if test "${LANGUAGE+set}"    = set; then LANGUAGE=C;    export LANGUAGE;    fi
if test "${LANG+set}"        = set; then LANG=C;        export LANG;        fi
if test "${LC_ALL+set}"      = set; then LC_ALL=C;      export LC_ALL;      fi
if test "${LC_MESSAGES+set}" = set; then LC_MESSAGES=C; export LC_MESSAGES; fi
if test "${LC_CTYPE+set}"    = set; then LC_CTYPE=C;    export LC_CTYPE;    fi
if test "${LC_COLLATE+set}"  = set; then LC_COLLATE=C;  export LC_COLLATE;  fi
#
AWK=${1-awk}
DATA=${2-../include/Caps}

data=data$$
trap 'rm -f $data' 1 2 5 15
sed -e 's/[	][	]*/	/g' < $DATA >$data
DATA=$data

echo "/*";
echo " * termsort.c --- sort order arrays for use by infocmp.";
echo " *";
echo " * Note: this file is generated using MKtermsort.sh, do not edit by hand.";
echo " */";

echo "static const PredIdx bool_terminfo_sort[] = {";
$AWK <$DATA '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "bool"    {printf("%s\t%d\n", $2, i++);}
' | sort | $AWK '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "static const PredIdx num_terminfo_sort[] = {";
$AWK <$DATA '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "num"     {printf("%s\t%d\n", $2, i++);}
' | sort | $AWK '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "static const PredIdx str_terminfo_sort[] = {";
$AWK <$DATA '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "str"     {printf("%s\t%d\n", $2, i++);}
' | sort | $AWK '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "static const PredIdx bool_variable_sort[] = {";
$AWK <$DATA '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "bool"    {printf("%s\t%d\n", $1, i++);}
' | sort | $AWK '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "static const PredIdx num_variable_sort[] = {";
$AWK <$DATA '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "num"     {printf("%s\t%d\n", $1, i++);}
' | sort | $AWK '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "static const PredIdx str_variable_sort[] = {";
$AWK <$DATA '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "str"     {printf("%s\t%d\n", $1, i++);}
' | sort | $AWK '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "static const PredIdx bool_termcap_sort[] = {";
$AWK <$DATA '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "bool"    {printf("%s\t%d\n", $4, i++);}
' | sort | $AWK '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "static const PredIdx num_termcap_sort[] = {";
$AWK <$DATA '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "num"     {printf("%s\t%d\n", $4, i++);}
' | sort | $AWK '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "static const PredIdx str_termcap_sort[] = {";
$AWK <$DATA '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "str"     {printf("%s\t%d\n", $4, i++);}
' | sort | $AWK '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "static const bool bool_from_termcap[] = {";
$AWK <$DATA '
$3 == "bool" && substr($7, 1, 1) == "-"       {print "\tFALSE,\t/* ", $2, " */";}
$3 == "bool" && substr($7, 1, 1) == "Y"       {print "\tTRUE,\t/* ", $2, " */";}
'
echo "};";
echo "";

echo "static const bool num_from_termcap[] = {";
$AWK <$DATA '
$3 == "num" && substr($7, 1, 1) == "-"        {print "\tFALSE,\t/* ", $2, " */";}
$3 == "num" && substr($7, 1, 1) == "Y"        {print "\tTRUE,\t/* ", $2, " */";}
'
echo "};";
echo "";

echo "static const bool str_from_termcap[] = {";
$AWK <$DATA '
$3 == "str" && substr($7, 1, 1) == "-"        {print "\tFALSE,\t/* ", $2, " */";}
$3 == "str" && substr($7, 1, 1) == "Y"        {print "\tTRUE,\t/* ", $2, " */";}
'
echo "};";
echo "";

rm -f $data