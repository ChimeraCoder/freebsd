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
SSL=openssl
if test $# -ge 1
then
  for i in $@
  do
  C=$i.pem
  test -f $C || C=$i
  if test -f $C
  then
    H=`$SSL x509 -noout -hash < $C`.0
    if test -h $H -o -f $H
    then
      echo link $H to $C exists
    else
      ln -s $C $H
    fi
  else
    echo "$0: cannot open $C"
    exit 2
  fi
  done
else
  echo "$0: missing name"
  exit 1
fi
exit 0