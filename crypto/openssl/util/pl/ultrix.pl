#!/usr/local/bin/perl
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
#
# linux.pl - the standard unix makefile stuff.
#
$o='/';
$cp='/bin/cp';
$rm='/bin/rm -f';

# C compiler stuff

$cc='cc';
if ($debug)
	{ $cflags="-g -DREF_CHECK -DCRYPTO_MDEBUG"; }
else
	{ $cflags="-O2"; }

$cflags.=" -std1 -DL_ENDIAN";

if (!$no_asm)
	{
	$bn_asm_obj='$(OBJ_D)/mips1.o';
	$bn_asm_src='crypto/bn/asm/mips1.s';
	}

sub do_link_rule
	{
	local($target,$files,$dep_libs,$libs)=@_;
	local($ret,$_);
	
	$file =~ s/\//$o/g if $o ne '/';
	$n=&bname($target);
	$ret.="$target: $files $dep_libs\n";
	$ret.="\t\$(LINK) ${efile}$target \$(LFLAGS) $files $libs\n\n";
	return($ret);
	}

1;