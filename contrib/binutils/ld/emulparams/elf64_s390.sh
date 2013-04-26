
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
# Treat a host that matches the target with the possible exception of "x"
# in the name as if it were native.if test `echo "$host" | sed -e s/390x/390/` \
   = `echo "$target" | sed -e s/390x/390/`; then
  case " $EMULATION_LIBPATH " in
    *" ${EMULATION_NAME} "*)
      NATIVE=yes
  esac
fi

# Look for 64 bit target libraries in /lib64, /usr/lib64 etc., first
# on Linux.
case "$target" in
  s390*-linux*)
    case "$EMULATION_NAME" in
      *64*)
	LIBPATH_SUFFIX=64 ;;
    esac
    ;;
esac