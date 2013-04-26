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
# Emulate nroff with groff.
# $FreeBSD$
prog="$0"
# Default device.
# First try the "locale charmap" command, because it's most reliable.
# On systems where it doesn't exist, look at the environment variables.
case "`exec 2>/dev/null ; locale charmap`" in
  UTF-8)
    T=-Tutf8 ;;
  ISO*8859-1 | ISO*8859-15)
    T=-Tlatin1 ;;
  KOI8-R)
    T=-Tkoi8-r ;;
  IBM-1047)
    T=-Tcp1047 ;;
  *)
    case "${LC_ALL-${LC_CTYPE-${LANG}}}" in
      *.UTF-8)
        T=-Tutf8 ;;
      iso_8859_1 | *.ISO-8859-1 | *.ISO8859-1 | \
      iso_8859_15 | *.ISO-8859-15 | *.ISO8859-15)
        T=-Tlatin1 ;;
      *.KOI8-R)
        T=-Tkoi8-r ;;
      *.IBM-1047)
        T=-Tcp1047 ;;
      *)
        case "$LESSCHARSET" in
          utf-8)
            T=-Tutf8 ;;
          latin1)
            T=-Tlatin1 ;;
          koi8-r)
            T=-Tkoi8-r ;;
          cp1047)
            T=-Tcp1047 ;;
          *)
            T=-Tascii ;;
          esac ;;
     esac ;;
esac
opts=

# `for i; do' doesn't work with some versions of sh

for i
  do
  case $1 in
    -c)
      opts="$opts -P-c" ;;
    -h)
      opts="$opts -P-h" ;;
    -[eq] | -s*)
      # ignore these options
      ;;
    -[dMmrnoT])
      echo "$prog: option $1 requires an argument" >&2
      exit 1 ;;
    -[iptSUC] | -[dMmrno]*)
      opts="$opts $1" ;;
    -Tascii | -Tlatin1 | -Tkoi8-r | -Tutf8 | -Tcp1047)
      T=$1 ;;
    -T*)
      # ignore other devices
      ;;
    -u*)
      # Solaris 2.2 through at least Solaris 9 `man' invokes
      # `nroff -u0 ... | col -x'.  Ignore the -u0,
      # since `less' and `more' can use the emboldening info.
      # However, disable SGR, since Solaris `col' mishandles it.
      opts="$opts -P-c" ;;
    -v | --version)
      echo "GNU nroff (groff) version @VERSION@"
      exit 0 ;;
    --help)
      echo "usage: nroff [-CchipStUv] [-dCS] [-MDIR] [-mNAME] [-nNUM] [-oLIST]"
      echo "             [-rCN] [-Tname] [FILE...]"
      exit 0 ;;
    --)
      shift
      break ;;
    -)
      break ;;
    -*)
      echo "$prog: invalid option $1" >&2
      exit 1 ;;
    *)
      break ;;
  esac
  shift
done

# Set up the `GROFF_BIN_PATH' variable
# to be exported in the current `GROFF_RUNTIME' environment.

@GROFF_BIN_PATH_SETUP@
export GROFF_BIN_PATH

# This shell script is intended for use with man, so warnings are
# probably not wanted.  Also load nroff-style character definitions.

PATH="$GROFF_RUNTIME$PATH" groff -mtty-char $T $opts ${1+"$@"}

# eof