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

echo Generating x86 assember
echo Bignum
(cd crypto/bn/asm; perl x86.pl cpp > bn86unix.cpp)
(cd crypto/bn/asm; perl x86.pl win32 > bn-win32.asm)

echo DES
(cd crypto/des/asm; perl des-586.pl cpp > dx86unix.cpp)
(cd crypto/des/asm; perl des-586.pl win32 > d-win32.asm)

echo "crypt(3)"
(cd crypto/des/asm; perl crypt586.pl cpp > yx86unix.cpp)
(cd crypto/des/asm; perl crypt586.pl win32 > y-win32.asm)

echo Blowfish
(cd crypto/bf/asm; perl bf-586.pl cpp > bx86unix.cpp)
(cd crypto/bf/asm; perl bf-586.pl win32 > b-win32.asm)

echo CAST5
(cd crypto/cast/asm; perl cast-586.pl cpp > cx86unix.cpp)
(cd crypto/cast/asm; perl cast-586.pl win32 > c-win32.asm)

echo RC4
(cd crypto/rc4/asm; perl rc4-586.pl cpp > rx86unix.cpp)
(cd crypto/rc4/asm; perl rc4-586.pl win32 > r4-win32.asm)

echo MD5
(cd crypto/md5/asm; perl md5-586.pl cpp > mx86unix.cpp)
(cd crypto/md5/asm; perl md5-586.pl win32 > m5-win32.asm)

echo SHA1
(cd crypto/sha/asm; perl sha1-586.pl cpp > sx86unix.cpp)
(cd crypto/sha/asm; perl sha1-586.pl win32 > s1-win32.asm)

echo RIPEMD160
(cd crypto/ripemd/asm; perl rmd-586.pl cpp > rm86unix.cpp)
(cd crypto/ripemd/asm; perl rmd-586.pl win32 > rm-win32.asm)

echo RC5/32
(cd crypto/rc5/asm; perl rc5-586.pl cpp > r586unix.cpp)
(cd crypto/rc5/asm; perl rc5-586.pl win32 > r5-win32.asm)