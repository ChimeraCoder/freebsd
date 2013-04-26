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

ECHO=echo
PKG_VERSION=./pkg_version

test-pv ( ) { \
    setvar v1 $1
    setvar answer $2
    setvar v2 $3
    setvar type $4
    res=`${PKG_VERSION} -t ${v1} ${v2}`
    if [ ${res} != ${answer} ]; then \
	${ECHO} "${type} test failed (${v1} ${res} ${v2}, should have been ${answer})"; \
    fi
}

# Test coercion of default PORTREVISION and PORTEPOCH
test-pv 0.10 "=" 0.10_0 coercion
test-pv 0.10 "=" 0.10,0 coercion
test-pv 0.10 "=" 0.10_0,0 coercion

# Test various comparisons
test-pv 1.0 "=" 1.0 equality
test-pv 2.15a "=" 2.15a equality

test-pv 0.10 ">" 0.9 inequality
test-pv 0.9 "<" 0.10 inequality

test-pv 2.3p10 ">" 2.3p9 number/letter
test-pv 1.6.0 ">" 1.6.0.p3 number/letter
test-pv 1.0.b ">" 1.0.a3 number/letter
test-pv 1.0a ">" 1.0 number/letter
test-pv 1.0a "<" 1.0b number/letter
test-pv 5.0a ">" 5.0.b number/letter

test-pv 1.5_1 ">" 1.5 portrevision
test-pv 1.5_2 ">" 1.5_1 portrevision
test-pv 1.5_1 "<" 1.5.0.1 portrevision
test-pv 1.5 "<" 1.5.0.1 portrevision

test-pv 00.01.01,1 ">" 99.12.31 portepoch
test-pv 0.0.1_1,2 ">" 0.0.1,2 portrevision/portepoch
test-pv 0.0.1_1,3 ">" 0.0.1_2,2 portrevision/portepoch

test-pv 2.0 ">" 2.a2 number/letter
test-pv 3 "=" 3.0 equality
test-pv 4a "<" 4a0 letter/zero
test-pv 10a1b2 "=" 10a1.b2 separator

test-pv 7pl "=" 7.pl patchevel
test-pv 8.0.a "=" 8.0alpha alpha
test-pv 9.b3.0 "=" 9beta3 beta
test-pv 10.pre7 "=" 10pre7.0 pre
test-pv 11.r "=" 11.rc rc

test-pv 12pl "<" 12alpha alpha/patchevel
test-pv 13.* "<" 13.pl star/patchevel

test-pv 1.0.0+2003.09.06 "=" 1.0+2003.09.06 plus/multiple
test-pv 1.0.1+2003.09.06 ">" 1.0+2003.09.06 plus/multiple
test-pv 1.0.0+2003.09.06 "<" 1.0+2003.09.06_1 plus/portrevision
test-pv 1.0.1+2003.09.06 ">" 1.0+2003.09.06_1 plus/portrevision