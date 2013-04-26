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

# We want to behave like make, any error forces us to stop.
set -e

action=$1
srcdir=$2
cgen="$3"
cgendir=$4
cgenflags=$5
arch=$6
prefix=$7
archfile=$8
opcfile=$9
shift ; options=$9

# List of extra files to build.
# Values: opinst (only 1 extra file at present)
shift ; extrafiles=$9

rootdir=${srcdir}/..

# $arch is $6, as passed on the command line.
# $ARCH is the same argument but in all uppercase.
# Both forms are used in this script.

lowercase='abcdefghijklmnopqrstuvwxyz'
uppercase='ABCDEFGHIJKLMNOPQRSTUVWXYZ'
ARCH=`echo ${arch} | tr "${lowercase}" "${uppercase}"`

extrafile_args=""
for ef in .. $extrafiles
do
    case $ef in
    ..) ;;
    opinst) extrafile_args="-Q tmp-opinst.c1 $extrafile_args" ;;
    esac
done

case $action in
opcodes)
	# Remove residual working files.
	rm -f tmp-desc.h tmp-desc.h1
	rm -f tmp-desc.c tmp-desc.c1
	rm -f tmp-opc.h tmp-opc.h1
	rm -f tmp-opc.c tmp-opc.c1
	rm -f tmp-opinst.c tmp-opinst.c1
	rm -f tmp-ibld.h tmp-ibld.h1
	rm -f tmp-ibld.c tmp-ibld.in1
	rm -f tmp-asm.c tmp-asm.in1
	rm -f tmp-dis.c tmp-dis.in1

	# Run CGEN.
	${cgen} ${cgendir}/cgen-opc.scm \
		-s ${cgendir} \
		${cgenflags} \
		-f "${options}" \
		-m all \
		-a ${archfile} \
	        -OPC ${opcfile} \
		-H tmp-desc.h1 \
		-C tmp-desc.c1 \
		-O tmp-opc.h1 \
		-P tmp-opc.c1 \
		-L tmp-ibld.in1 \
		-A tmp-asm.in1 \
		-D tmp-dis.in1 \
		${extrafile_args}

	# Customise generated files for the particular architecture.
	sed -e "s/@ARCH@/${ARCH}/g" -e "s/@arch@/${arch}/g" < tmp-desc.h1 > tmp-desc.h
	${rootdir}/move-if-change tmp-desc.h ${srcdir}/${prefix}-desc.h

	sed -e "s/@ARCH@/${ARCH}/g" -e "s/@arch@/${arch}/g" \
		-e "s/@prefix@/${prefix}/" < tmp-desc.c1 > tmp-desc.c
	${rootdir}/move-if-change tmp-desc.c ${srcdir}/${prefix}-desc.c

	sed -e "s/@ARCH@/${ARCH}/g" -e "s/@arch@/${arch}/g" < tmp-opc.h1 > tmp-opc.h
	${rootdir}/move-if-change tmp-opc.h ${srcdir}/${prefix}-opc.h

	sed -e "s/@ARCH@/${ARCH}/g" -e "s/@arch@/${arch}/g" \
		-e "s/@prefix@/${prefix}/" < tmp-opc.c1 > tmp-opc.c
	${rootdir}/move-if-change tmp-opc.c ${srcdir}/${prefix}-opc.c

	case $extrafiles in
	*opinst*)
	  sed -e "s/@ARCH@/${ARCH}/g" -e "s/@arch@/${arch}/g" \
		-e "s/@prefix@/${prefix}/" < tmp-opinst.c1 >tmp-opinst.c
	  ${rootdir}/move-if-change tmp-opinst.c ${srcdir}/${prefix}-opinst.c
	  ;;
	esac

	cat ${srcdir}/cgen-ibld.in tmp-ibld.in1 | \
	  sed -e "s/@ARCH@/${ARCH}/g" -e "s/@arch@/${arch}/g" \
		-e "s/@prefix@/${prefix}/" > tmp-ibld.c
	${rootdir}/move-if-change tmp-ibld.c ${srcdir}/${prefix}-ibld.c

	sed -e "/ -- assembler routines/ r tmp-asm.in1" ${srcdir}/cgen-asm.in \
	  | sed -e "s/@ARCH@/${ARCH}/g" -e "s/@arch@/${arch}/g" \
		-e "s/@prefix@/${prefix}/" > tmp-asm.c
	${rootdir}/move-if-change tmp-asm.c ${srcdir}/${prefix}-asm.c

	sed -e "/ -- disassembler routines/ r tmp-dis.in1" ${srcdir}/cgen-dis.in \
	  | sed -e "s/@ARCH@/${ARCH}/g" -e "s/@arch@/${arch}/g" \
		-e "s/@prefix@/${prefix}/" > tmp-dis.c
	${rootdir}/move-if-change tmp-dis.c ${srcdir}/${prefix}-dis.c

	# Remove temporary files.
	rm -f tmp-desc.h1 tmp-desc.c1
	rm -f tmp-opc.h1 tmp-opc.c1
	rm -f tmp-opinst.c1
	rm -f tmp-ibld.h1 tmp-ibld.in1
	rm -f tmp-asm.in1 tmp-dis.in1
	;;

*)
	echo "$0: bad action: ${action}" >&2
	exit 1
	;;

esac

exit 0