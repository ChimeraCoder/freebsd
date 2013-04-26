#!/usr/bin/env perl
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

$0 =~ m/(.*[\/\\])[^\/\\]+$/; $dir=$1;
push(@INC,"${dir}","${dir}../../perlasm");
require "x86asm.pl";

&asm_init($ARGV[0],"via-mont.pl");

# int bn_mul_mont(BN_ULONG *rp, const BN_ULONG *ap, const BN_ULONG *bp, const BN_ULONG *np,const BN_ULONG *n0, int num);
$func="bn_mul_mont_padlock";

$pad=16*1;	# amount of reserved bytes on top of every vector

# stack layout
$mZeroPrime=&DWP(0,"esp");		# these are specified by VIA
$A=&DWP(4,"esp");
$B=&DWP(8,"esp");
$T=&DWP(12,"esp");
$M=&DWP(16,"esp");
$scratch=&DWP(20,"esp");
$rp=&DWP(24,"esp");			# these are mine
$sp=&DWP(28,"esp");
# &DWP(32,"esp")			# 32 byte scratch area
# &DWP(64+(4*$num+$pad)*0,"esp")	# padded tp[num]
# &DWP(64+(4*$num+$pad)*1,"esp")	# padded copy of ap[num]
# &DWP(64+(4*$num+$pad)*2,"esp")	# padded copy of bp[num]
# &DWP(64+(4*$num+$pad)*3,"esp")	# padded copy of np[num]
# Note that SDK suggests to unconditionally allocate 2K per vector. This
# has quite an impact on performance. It naturally depends on key length,
# but to give an example 1024 bit private RSA key operations suffer >30%
# penalty. I allocate only as much as actually required...

&function_begin($func);
	&xor	("eax","eax");
	&mov	("ecx",&wparam(5));	# num
	# meet VIA's limitations for num [note that the specification
	# expresses them in bits, while we work with amount of 32-bit words]
	&test	("ecx",3);
	&jnz	(&label("leave"));	# num % 4 != 0
	&cmp	("ecx",8);
	&jb	(&label("leave"));	# num < 8
	&cmp	("ecx",1024);
	&ja	(&label("leave"));	# num > 1024

	&pushf	();
	&cld	();

	&mov	("edi",&wparam(0));	# rp
	&mov	("eax",&wparam(1));	# ap
	&mov	("ebx",&wparam(2));	# bp
	&mov	("edx",&wparam(3));	# np
	&mov	("esi",&wparam(4));	# n0
	&mov	("esi",&DWP(0,"esi"));	# *n0

	&lea	("ecx",&DWP($pad,"","ecx",4));	# ecx becomes vector size in bytes
	&lea	("ebp",&DWP(64,"","ecx",4));	# allocate 4 vectors + 64 bytes
	&neg	("ebp");
	&add	("ebp","esp");
	&and	("ebp",-64);		# align to cache-line
	&xchg	("ebp","esp");		# alloca

	&mov	($rp,"edi");		# save rp
	&mov	($sp,"ebp");		# save esp

	&mov	($mZeroPrime,"esi");
	&lea	("esi",&DWP(64,"esp"));	# tp
	&mov	($T,"esi");
	&lea	("edi",&DWP(32,"esp"));	# scratch area
	&mov	($scratch,"edi");
	&mov	("esi","eax");

	&lea	("ebp",&DWP(-$pad,"ecx"));
	&shr	("ebp",2);		# restore original num value in ebp

	&xor	("eax","eax");

	&mov	("ecx","ebp");
	&lea	("ecx",&DWP((32+$pad)/4,"ecx"));# padded tp + scratch
	&data_byte(0xf3,0xab);		# rep stosl, bzero

	&mov	("ecx","ebp");
	&lea	("edi",&DWP(64+$pad,"esp","ecx",4));# pointer to ap copy
	&mov	($A,"edi");
	&data_byte(0xf3,0xa5);		# rep movsl, memcpy
	&mov	("ecx",$pad/4);
	&data_byte(0xf3,0xab);		# rep stosl, bzero pad
	# edi points at the end of padded ap copy...

	&mov	("ecx","ebp");
	&mov	("esi","ebx");
	&mov	($B,"edi");
	&data_byte(0xf3,0xa5);		# rep movsl, memcpy
	&mov	("ecx",$pad/4);
	&data_byte(0xf3,0xab);		# rep stosl, bzero pad
	# edi points at the end of padded bp copy...

	&mov	("ecx","ebp");
	&mov	("esi","edx");
	&mov	($M,"edi");
	&data_byte(0xf3,0xa5);		# rep movsl, memcpy
	&mov	("ecx",$pad/4);
	&data_byte(0xf3,0xab);		# rep stosl, bzero pad
	# edi points at the end of padded np copy...

	# let magic happen...
	&mov	("ecx","ebp");
	&mov	("esi","esp");
	&shl	("ecx",5);		# convert word counter to bit counter
	&align	(4);
	&data_byte(0xf3,0x0f,0xa6,0xc0);# rep montmul

	&mov	("ecx","ebp");
	&lea	("esi",&DWP(64,"esp"));		# tp
	# edi still points at the end of padded np copy...
	&neg	("ebp");
	&lea	("ebp",&DWP(-$pad,"edi","ebp",4));	# so just "rewind"
	&mov	("edi",$rp);			# restore rp
	&xor	("edx","edx");			# i=0 and clear CF

&set_label("sub",8);
	&mov	("eax",&DWP(0,"esi","edx",4));
	&sbb	("eax",&DWP(0,"ebp","edx",4));
	&mov	(&DWP(0,"edi","edx",4),"eax");	# rp[i]=tp[i]-np[i]
	&lea	("edx",&DWP(1,"edx"));		# i++
	&loop	(&label("sub"));		# doesn't affect CF!

	&mov	("eax",&DWP(0,"esi","edx",4));	# upmost overflow bit
	&sbb	("eax",0);
	&and	("esi","eax");
	&not	("eax");
	&mov	("ebp","edi");
	&and	("ebp","eax");
	&or	("esi","ebp");			# tp=carry?tp:rp

	&mov	("ecx","edx");			# num
	&xor	("edx","edx");			# i=0

&set_label("copy",8);
	&mov	("eax",&DWP(0,"esi","edx",4));
	&mov	(&DWP(64,"esp","edx",4),"ecx");	# zap tp
	&mov	(&DWP(0,"edi","edx",4),"eax");
	&lea	("edx",&DWP(1,"edx"));		# i++
	&loop	(&label("copy"));

	&mov	("ebp",$sp);
	&xor	("eax","eax");

	&mov	("ecx",64/4);
	&mov	("edi","esp");		# zap frame including scratch area
	&data_byte(0xf3,0xab);		# rep stosl, bzero

	# zap copies of ap, bp and np
	&lea	("edi",&DWP(64+$pad,"esp","edx",4));# pointer to ap
	&lea	("ecx",&DWP(3*$pad/4,"edx","edx",2));
	&data_byte(0xf3,0xab);		# rep stosl, bzero

	&mov	("esp","ebp");
	&inc	("eax");		# signal "done"
	&popf	();
&set_label("leave");
&function_end($func);

&asciz("Padlock Montgomery Multiplication, CRYPTOGAMS by <appro\@openssl.org>");

&asm_finish();