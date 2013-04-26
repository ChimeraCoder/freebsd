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
# Provision of this shell script should not be taken to imply that use of
# GNU eqn with groff -Tascii|-Tlatin1|-Tkoi8-r|-Tutf8|-Tcp1047 is supported.
# $FreeBSD$
# Default device.
case "${LC_ALL-${LC_CTYPE-${LANG}}}" in
  *.UTF-8)
    T=utf8 ;;
  iso_8859_1 | *.ISO*8859-1 | *.ISO*8859-15)
    T=latin1 ;;
  *.IBM-1047)
    T=cp1047 ;;
  *.KOI8-R)
    T=koi8-r ;;
  *)
    T=ascii ;;
esac

@GROFF_BIN_PATH_SETUP@
PATH="$GROFF_RUNTIME$PATH"
export PATH
exec @g@eqn -T${T} ${1+"$@"}

# eof