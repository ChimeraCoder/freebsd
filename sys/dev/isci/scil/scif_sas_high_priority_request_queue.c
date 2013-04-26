
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
 * @brief This file contains all of method implementations specific to
 *        the high priority request queue (HPRQ).  See the
 *        scif_sas_high_priority_request_queue.h file for additional
 *        information.
 */

#include <dev/isci/scil/scif_sas_high_priority_request_queue.h>

#include <dev/isci/scil/scif_sas_logger.h>
#include <dev/isci/scil/scif_sas_io_request.h>
#include <dev/isci/scil/scif_sas_controller.h>

/**
 * @brief This method initializes the fields of the HPRQ.  It should be
 *        called during the construction of the object containing or
 *        utilizing it (i.e. SCIF_SAS_CONTROLLER).
 *
 * @param[in] fw_hprq This parameter specific the high priority request
 *            queue object being constructed.
 *
 * @return none
 */
void scif_sas_high_priority_request_queue_construct(
   SCIF_SAS_HIGH_PRIORITY_REQUEST_QUEUE_T * fw_hprq,
   SCI_BASE_LOGGER_T                      * logger
)
{
   SCIF_LOG_TRACE((
      logger,
      SCIF_LOG_OBJECT_CONTROLLER | SCIF_LOG_OBJECT_DOMAIN_DISCOVERY,
      "scif_sas_high_priority_request_queue_construct(0x%x,0x%x) enter\n",
      fw_hprq, logger
   ));

   sci_base_object_construct((SCI_BASE_OBJECT_T*) &fw_hprq->lock, logger);
   fw_hprq->lock.level = SCI_LOCK_LEVEL_NONE;

   sci_pool_initialize(fw_hprq->pool);
}

/**
 * @brief This method will ensure all internal requests destined for
 *        devices contained in the supplied domain are properly removed
 *        from the high priority request queue.
 *
 * @param[in] fw_hprq This parameter specifies the high priority request
 *            queue object for which to attempt to remove elements.
 * @param[in] fw_domain This parameter specifies the domain for which to
 *            remove all high priority requests.
 *
 * @return none
 */
void scif_sas_high_priority_request_queue_purge_domain(
   SCIF_SAS_HIGH_PRIORITY_REQUEST_QUEUE_T * fw_hprq,
   SCIF_SAS_DOMAIN_T                      * fw_domain
)
{
   SCIF_SAS_IO_REQUEST_T * fw_io;
   POINTER_UINT            io_address;
   U32                     index;
   U32                     element_count;

   SCIF_LOG_TRACE((
      sci_base_object_get_logger(&fw_hprq->lock),
      SCIF_LOG_OBJECT_CONTROLLER | SCIF_LOG_OBJECT_DOMAIN_DISCOVERY,
      "scif_sas_high_priority_request_queue_purge_domain(0x%x,0x%x) enter\n",
      fw_hprq, fw_domain
   ));

   element_count = sci_pool_count(fw_hprq->pool);

   scif_cb_lock_acquire(fw_domain->controller, &fw_hprq->lock);

   for (index = 0; index < element_count; index++)
   {
      sci_pool_get(fw_hprq->pool, io_address);

      fw_io = (SCIF_SAS_IO_REQUEST_T*) io_address;

      // If the high priority request is not intended for this domain,
      // then it can be left in the pool.
      if (fw_io->parent.device->domain != fw_domain)
      {
         sci_pool_put(fw_hprq->pool, io_address);
      }
      else
      {
         if (fw_io->parent.is_internal)
         {
            SCIF_SAS_INTERNAL_IO_REQUEST_T * fw_internal_io =
               (SCIF_SAS_INTERNAL_IO_REQUEST_T *)fw_io;

            // The request was intended for a device in the domain.  Put it
            // back in the pool of freely available internal request memory
            // objects. The internal IO's timer is to be destoyed.
            scif_sas_internal_io_request_destruct(fw_domain->controller, fw_internal_io);
         }
      }
   }

   scif_cb_lock_release(fw_domain->controller, &fw_hprq->lock);
}