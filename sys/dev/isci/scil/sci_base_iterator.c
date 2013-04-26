
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/**
 * @file
 *
 * @brief This file contains the implementation of an iterator class.
 *        This class will allow for iterating across the elements of a
 *        container.
 */

#if !defined(DISABLE_SCI_ITERATORS)

//******************************************************************************
//*
//*     I N C L U D E S
//*
//******************************************************************************

#include <dev/isci/scil/sci_base_iterator.h>

//******************************************************************************
//*
//*     P R I V A T E   M E M B E R S
//*
//******************************************************************************

//******************************************************************************
//*
//*     P R O T E C T E D   M E T H O D S
//*
//******************************************************************************

/**
 * @brief Return the size of an iterator object.
 *
 * @return U32 : size of iterator object in bytes.
 *
 */
U32 sci_iterator_get_object_size(
   void
)
{
    return sizeof(SCI_BASE_ITERATOR_T);
}

/**
 * @brief Initialize the interator.
 *
 * @param[in] iterator This parameter specifies the iterator to be
 *            constructed.
 * @param[in] list This parameter specifies the abstract list that will be
 *            iterated on by this iterator.  The iterator will by initialized
 *            to point to the first element in this abstract list.
 *
 * @return none
 */
void sci_base_iterator_construct(
   SCI_ITERATOR_HANDLE_T   iterator_handle,
   SCI_ABSTRACT_LIST_T   * list
)
{
    SCI_BASE_ITERATOR_T * iterator = (SCI_BASE_ITERATOR_T *) iterator_handle;

    memset(iterator, 0, sizeof(SCI_BASE_ITERATOR_T));
    iterator->list = list;
    sci_iterator_first(iterator);
}

/**
 * @brief Get the object currently pointed to by this iterator.
 *
 * @param[in] iterator_handle Handle to an iterator.
 *
 * @return void * : Object pointed to by this iterator.
 * @retval NULL If iterator is not currently pointing to a valid element.
 */
void * sci_iterator_get_current(
   SCI_ITERATOR_HANDLE_T iterator_handle
)
{
   SCI_BASE_ITERATOR_T * iterator = (SCI_BASE_ITERATOR_T *)iterator_handle;

   void *current_object = NULL;

   if (iterator->current != NULL)
   {
      current_object = sci_abstract_list_get_object(iterator->current);
   }

   return current_object;
}

/**
 * @brief Modify the iterator to point to the first element in the list.
 *
 * @param[in] iterator
 *
 * @return none
 */
void sci_iterator_first(
   SCI_ITERATOR_HANDLE_T iterator_handle
)
{
   SCI_BASE_ITERATOR_T * iterator = (SCI_BASE_ITERATOR_T *)iterator_handle;

   iterator->current = sci_abstract_list_get_front(iterator->list);
}

/**
 * @brief Modify the iterator to point to the next element in the list.
 *
 * @param[in] iterator
 *
 * @return none
 */
void sci_iterator_next(
   SCI_ITERATOR_HANDLE_T iterator_handle
)
{
   SCI_BASE_ITERATOR_T * iterator = (SCI_BASE_ITERATOR_T *)iterator_handle;

   if (iterator->current != NULL)
   {
      iterator->current = sci_abstract_list_get_next(iterator->current);
   }
}

#endif // !defined(DISABLE_SCI_ITERATORS)