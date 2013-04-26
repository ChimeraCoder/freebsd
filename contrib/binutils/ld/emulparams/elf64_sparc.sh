
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
# Treat a host that matches the target with the possible exception of "64"
# and "v7", "v8", "v9" in the name as if it were native.if test `echo "$host" | sed -e 's/64//;s/v[789]//'` \
 = `echo "$target" | sed -e 's/64//;s/v[789]//'`; then
  case " $EMULATION_LIBPATH " in
    *" ${EMULATION_NAME} "*)
      NATIVE=yes
      ;;
  esac
fi

# Linux and Solaris modify the default library search path
# to first include a 64-bit specific directory.  It's put
# in slightly different places on the two systems.
# Look for 64 bit target libraries in /lib64, /usr/lib64 etc., first
# on Linux and /lib/sparcv9, /usr/lib/sparcv9 etc. on Solaris.
case "$EMULATION_NAME" in
  *64*)
    case "$target" in
      sparc*-linux*)
	LIBPATH_SUFFIX=64 ;;
      sparc*-solaris*)
	LIBPATH_SUFFIX=/sparcv9 ;;
    esac
    ;;
esac