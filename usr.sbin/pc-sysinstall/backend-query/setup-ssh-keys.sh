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

# Script which sets up password-less logins for ssh host
###########################################################################

. ${PROGDIR}/backend/functions.sh

SSHUSER=$1
SSHHOST=$2
SSHPORT=$3

if [ -z "${SSHUSER}" -o -z "${SSHHOST}" -o -z "${SSHPORT}" ]
then
  echo "ERROR: Usage setup-ssh-keys <user> <host> <port>"
  exit 150
fi

cd ~

echo "Preparing to setup SSH key authorization..."
echo "When prompted, enter your password for ${SSHUSER}@${SSHHOST}"

if [ ! -e ".ssh/id_rsa.pub" ]
then
  mkdir .ssh >/dev/null 2>/dev/null
  ssh-keygen -q -t rsa -N '' -f .ssh/id_rsa
  sleep 1
fi

if [ ! -e ".ssh/id_rsa.pub" ]
then
  echo "ERROR: Failed creating .ssh/id_rsa.pub"
  exit 150
fi

# Get the .pub key
PUBKEY="`cat .ssh/id_rsa.pub`"

ssh -p ${SSHPORT} ${SSHUSER}@${SSHHOST} "mkdir .ssh ; echo $PUBKEY >> .ssh/authorized_keys; chmod 600 .ssh/authorized_keys ; echo $PUBKEY >> .ssh/authorized_keys2; chmod 600 .ssh/authorized_keys2"