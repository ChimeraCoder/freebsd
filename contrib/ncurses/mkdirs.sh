#! /bin/sh
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
# $Id: mkdirs.sh,v 1.5 2007/03/25 22:29:46 tom Exp $
# -----------------------------------------------------------------------------
# mkinstalldirs --- make directory hierarchy
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-05-16
# Last modified: 1994-03-25
# Public domain
# -----------------------------------------------------------------------------
errstatus=0
umask 022

for file in ${1+"$@"} ; do
   set fnord `echo ":$file" | sed -ne 's/^:\//#/;s/^://;s/\// /g;s/^#/\//;p'`
   shift

   pathcomp=
   for d in ${1+"$@"} ; do
     pathcomp="$pathcomp$d"
     case "$pathcomp" in
       -* ) pathcomp=./$pathcomp ;;
     esac

     if test ! -d "$pathcomp"; then
        echo "mkdir $pathcomp" 1>&2
        case "$pathcomp" in
          [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ]: )
            ;;               # DOSISH systems
          * )
            mkdir "$pathcomp"
            errstatus=$?
            if test $errstatus != 0
            then
               # may have failed if invoked in a parallel "make -j# install"
               if test -d "$pathcomp"
               then
                  errstatus=0
               fi
            fi
            ;;
        esac
     fi

     pathcomp="$pathcomp/"
   done
done

exit $errstatus

# mkinstalldirs ends here