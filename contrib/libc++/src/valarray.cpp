
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "valarray"

_LIBCPP_BEGIN_NAMESPACE_STD

template valarray<size_t>::valarray(size_t);
template valarray<size_t>::~valarray();
template void valarray<size_t>::resize(size_t, size_t);

void
gslice::__init(size_t __start)
{
    valarray<size_t> __indices(__size_.size());
    size_t __k = __size_.size() != 0;
    for (size_t __i = 0; __i < __size_.size(); ++__i)
        __k *= __size_[__i];
    __1d_.resize(__k);
    if (__1d_.size())
    {
        __k = 0;
        __1d_[__k] = __start;
        while (true)
        {
            size_t __i = __indices.size() - 1;
            while (true)
            {
                if (++__indices[__i] < __size_[__i])
                {
                    ++__k;
                    __1d_[__k] = __1d_[__k-1] + __stride_[__i];
                    for (size_t __j = __i + 1; __j != __indices.size(); ++__j)
                        __1d_[__k] -= __stride_[__j] * (__size_[__j] - 1);
                    break;
                }
                else
                {
                    if (__i == 0)
                        return;
                    __indices[__i--] = 0;
                }
            }
        }
    }
}

_LIBCPP_END_NAMESPACE_STD