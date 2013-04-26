
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
# version=Protocol.Major.Minor
# odd minor numbers are for -dev, even minor numbers are for -stableproto=4
major=2
minor=4
version=${proto}.${major}.${minor}
# Point.  3 cases:
# - Numeric values increment
# - empty 'increments' to 1
# - NEW 'increments' to empty
point=8
# Special.  Normally unused.  A suffix.
#special=ag
special=
# [???] To start an RC cycle: RC->yes
# To release from an RC cycle: rcpoint-> GO
# ReleaseCandidate. 'yes' or 'no'.
#releasecandidate=yes
releasecandidate=no
# ChangeLog tag
CLTAG=NTP_4_2_0
###
# The following is for ntp-stable.  2 cases:
# - Numeric values increment
# - GO triggers a release (from releasecandidate=yes)
# - - rcpoint gets set to 0
# - - releasecandidate gets set to no
# - GRONK is for -dev
rcpoint=
#rcpoint=GRONK