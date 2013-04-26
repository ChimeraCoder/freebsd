
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
 * @brief This file contains the method implementations for the
 *        SCIF_SAS_SMP_IO_REQUEST object.  The contents will implement SMP
 *        specific functionality.
 */

#include <dev/isci/scil/scif_sas_smp_io_request.h>
#include <dev/isci/scil/scif_sas_logger.h>
#include <dev/isci/scil/scif_sas_controller.h>
#include <dev/isci/scil/sci_controller.h>

#include <dev/isci/scil/sci_status.h>
#include <dev/isci/scil/scic_io_request.h>
#include <dev/isci/scil/scic_user_callback.h>

#include <dev/isci/scil/intel_sas.h>

/**
 * @brief This routine is to fill in the space given by core the SMP command
 *        frame. Then it calls core's construction.
 *
 * @param[in] fw_io The smp io request to be constructed.
 * @param[in] smp_command The SMP request filled according to SAS spec.
 *
 * @return none
 */
void scif_sas_smp_request_construct(
   SCIF_SAS_REQUEST_T * fw_request,
   SMP_REQUEST_T * smp_command
)
{
   void * command_iu_address =
      scic_io_request_get_command_iu_address(fw_request->core_object);

   //copy the smp_command to the address;
   memcpy( (char*) command_iu_address,
           smp_command,
           sizeof(SMP_REQUEST_T)
          );

   scic_io_request_construct_smp(fw_request->core_object);

   fw_request->protocol_complete_handler
      = NULL;
}

/**
 * @brief This method will perform all of the construction common to all
 *        SMP requests (e.g. filling in the frame type, zero-out memory,
 *        etc.).
 *
 * @param[out] smp_request This parameter specifies the SMP request
 *             structure containing the SMP request to be sent to the
 *             SMP target.
 * @param[in]  smp_function This parameter specifies the SMP function to
 *             sent.
 * @param[in]  smp_response_length This parameter specifies the length of
 *             the response (in DWORDs) that will be returned for this
 *             SMP request.
 * @param[in]  smp_request_length This parameter specifies the length of
 *             the request (in DWORDs) that will be sent.
 */
static
void scif_sas_smp_protocol_request_construct(
   SMP_REQUEST_T * smp_request,
   U8              smp_function,
   U8              smp_response_length,
   U8              smp_request_length
)
{
   memset((char*)smp_request, 0, sizeof(SMP_REQUEST_T));

   smp_request->header.smp_frame_type            = SMP_FRAME_TYPE_REQUEST;
   smp_request->header.function                  = smp_function;
   smp_request->header.allocated_response_length = smp_response_length;
   smp_request->header.request_length            = smp_request_length;
}


/**
 * @brief This method will allocate the internal IO request object and
 *        construct its contents based upon the supplied SMP request.
 *
 * @param[in] fw_controller This parameter specifies the controller object
 *            from which to allocate the internal IO request.
 * @param[in] fw_device This parameter specifies the remote device for
 *            which the internal IO request is destined.
 * @param[in] smp_request This parameter specifies the SMP request contents
 *            to be sent to the SMP target.
 *
 * @return void * The address of built scif sas smp request.
 */
static
void * scif_sas_smp_request_build(
   SCIF_SAS_CONTROLLER_T    * fw_controller,
   SCIF_SAS_REMOTE_DEVICE_T * fw_device,
   SMP_REQUEST_T            * smp_request,
   void                     * external_request_object,
   void                     * external_memory
)
{
   if (external_memory != NULL && external_request_object != NULL)
   {
      scif_sas_io_request_construct_smp(
         fw_controller,
         fw_device,
         external_memory,
         (char *)external_memory + sizeof(SCIF_SAS_IO_REQUEST_T),
         SCI_CONTROLLER_INVALID_IO_TAG,
         smp_request,
         external_request_object
      );

      return external_memory;
   }
   else
   {
      void * internal_io_memory;
      internal_io_memory = scif_sas_controller_allocate_internal_request(fw_controller);
      ASSERT(internal_io_memory != NULL);

      if (internal_io_memory != NULL)
      {
         //construct, only when we got valid io memory.
         scif_sas_internal_io_request_construct_smp(
            fw_controller,
            fw_device,
            internal_io_memory,
            SCI_CONTROLLER_INVALID_IO_TAG,
            smp_request
         );
      }
      else
      {
         SCIF_LOG_ERROR((
            sci_base_object_get_logger(fw_controller),
            SCIF_LOG_OBJECT_IO_REQUEST,
            "scif_sas_smp_request_build, no memory available!\n"
         ));
      }

      return internal_io_memory;
   }
}

/**
 * @brief construct a smp Report Genernal command to the fw_device.
 *
 * @param[in] fw_controller The framework controller object.
 * @param[in] fw_device the framework device that the REPORT GENERAL command
 *       targets to.
 *
 * @return void * address to the built scif sas smp request.
 */
void * scif_sas_smp_request_construct_report_general(
   SCIF_SAS_CONTROLLER_T    * fw_controller,
   SCIF_SAS_REMOTE_DEVICE_T * fw_device
)
{
   SMP_REQUEST_T smp_report_general;

   // Build the REPORT GENERAL request.
   scif_sas_smp_protocol_request_construct(
      &smp_report_general,
      SMP_FUNCTION_REPORT_GENERAL,
      sizeof(SMP_RESPONSE_REPORT_GENERAL_T) / sizeof(U32),
      0
   );

   smp_report_general.request.report_general.crc = 0;

   SCIF_LOG_INFO((
      sci_base_object_get_logger(fw_device),
      SCIF_LOG_OBJECT_IO_REQUEST | SCIF_LOG_OBJECT_DOMAIN_DISCOVERY,
      "SMP REPORT GENERAL -  Device:0x%x\n",
      fw_device
   ));

   return scif_sas_smp_request_build(
             fw_controller, fw_device, &smp_report_general, NULL, NULL);
}

/**
 * @brief construct a SMP Report Manufacturer Info request to the fw_device.
 *
 * @param[in] fw_controller The framework controller object.
 * @param[in] fw_device the framework device that the REPORT MANUFACTURER
 *            INFO targets to.
 *
 * @return void * address to the built scif sas smp request.
 */
void * scif_sas_smp_request_construct_report_manufacturer_info(
   SCIF_SAS_CONTROLLER_T    * fw_controller,
   SCIF_SAS_REMOTE_DEVICE_T * fw_device
)
{
   SMP_REQUEST_T smp_report_manufacturer_info;

   scif_sas_smp_protocol_request_construct(
      &smp_report_manufacturer_info,
      SMP_FUNCTION_REPORT_MANUFACTURER_INFORMATION,
      sizeof(SMP_RESPONSE_REPORT_MANUFACTURER_INFORMATION_T) / sizeof(U32),
      0
   );

   smp_report_manufacturer_info.request.report_general.crc = 0;

   SCIF_LOG_INFO((
      sci_base_object_get_logger(fw_device),
      SCIF_LOG_OBJECT_IO_REQUEST | SCIF_LOG_OBJECT_DOMAIN_DISCOVERY,
      "SMP REPORT MANUFACTURER_INFO -  Device:0x%x\n",
      fw_device
   ));

   return scif_sas_smp_request_build(
             fw_controller, fw_device, &smp_report_manufacturer_info, NULL, NULL
          );
}

/**
 * @brief construct a smp Discover command to the fw_device.
 * @param[in] fw_controller The framework controller object.
 * @param[in] fw_device the framework smp device that DISCOVER command targets
 *       to.
 * @param[in] phy_identifier The phy index the DISCOVER command targets to.
 *
 * @return void * address to the built scif sas smp request.
 */
void * scif_sas_smp_request_construct_discover(
   SCIF_SAS_CONTROLLER_T    * fw_controller,
   SCIF_SAS_REMOTE_DEVICE_T * fw_device,
   U8                         phy_identifier,
   void                     * external_request_object,
   void                     * external_memory
)
{
   SMP_REQUEST_T smp_discover;

   scif_sas_smp_protocol_request_construct(
      &smp_discover,
      SMP_FUNCTION_DISCOVER,
      sizeof(SMP_RESPONSE_DISCOVER_T) / sizeof(U32),
      sizeof(SMP_REQUEST_PHY_IDENTIFIER_T) / sizeof(U32)
   );

   smp_discover.request.discover.phy_identifier = phy_identifier;

   SCIF_LOG_INFO((
      sci_base_object_get_logger(fw_device),
      SCIF_LOG_OBJECT_IO_REQUEST | SCIF_LOG_OBJECT_DOMAIN_DISCOVERY,
      "SMP DISCOVER - Device:0x%x PhyId:0x%x\n",
      fw_device, phy_identifier
   ));

   return scif_sas_smp_request_build(
             fw_controller, fw_device, &smp_discover,
             external_request_object, external_memory
          );
}


/**
 * @brief construct a smp REPORT PHY SATA command to the fw_device.
 * @param[in] fw_controller The framework controller object.
 * @param[in] fw_device the framework smp device that DISCOVER command targets
 *       to.
 * @param[in] phy_identifier The phy index the DISCOVER command targets to.
 *
 * @return void * address to the built scif sas smp request.
 */
void * scif_sas_smp_request_construct_report_phy_sata(
   SCIF_SAS_CONTROLLER_T    * fw_controller,
   SCIF_SAS_REMOTE_DEVICE_T * fw_device,
   U8                         phy_identifier
)
{
   SMP_REQUEST_T report_phy_sata;

   scif_sas_smp_protocol_request_construct(
      &report_phy_sata,
      SMP_FUNCTION_REPORT_PHY_SATA,
      sizeof(SMP_RESPONSE_REPORT_PHY_SATA_T) / sizeof(U32),
      sizeof(SMP_REQUEST_PHY_IDENTIFIER_T) / sizeof(U32)
   );

   report_phy_sata.request.report_phy_sata.phy_identifier = phy_identifier;

   SCIF_LOG_INFO((
      sci_base_object_get_logger(fw_device),
      SCIF_LOG_OBJECT_IO_REQUEST | SCIF_LOG_OBJECT_DOMAIN_DISCOVERY,
      "SMP REPORT PHY SATA - Device:0x%x PhyId:0x%x\n",
      fw_device, phy_identifier
   ));

   return scif_sas_smp_request_build(
             fw_controller, fw_device, &report_phy_sata, NULL, NULL);
}


/**
 * @brief construct a smp REPORT PHY SATA command to the fw_device.
 * @param[in] fw_controller The framework controller object.
 * @param[in] fw_device the framework smp device that PHY CONTROL command
 *       targets to.
 * @param[in] phy_identifier The phy index the DISCOVER command targets to.
 *
 * @return void * address to the built scif sas smp request.
 */
void * scif_sas_smp_request_construct_phy_control(
   SCIF_SAS_CONTROLLER_T    * fw_controller,
   SCIF_SAS_REMOTE_DEVICE_T * fw_device,
   U8                         phy_operation,
   U8                         phy_identifier,
   void                     * external_request_object,
   void                     * external_memory
)
{
   SMP_REQUEST_T phy_control;

   scif_sas_smp_protocol_request_construct(
      &phy_control,
      SMP_FUNCTION_PHY_CONTROL,
      0,
      sizeof(SMP_REQUEST_PHY_CONTROL_T) / sizeof(U32)
   );

   phy_control.request.phy_control.phy_operation = phy_operation;
   phy_control.request.phy_control.phy_identifier = phy_identifier;

   return scif_sas_smp_request_build(
             fw_controller, fw_device, &phy_control,
             external_request_object, external_memory
          );
}


/**
 * @brief construct a smp CONFIG ROUTE INFO command to the fw_device.
 *
 * @param[in] fw_controller The framework controller object.
 * @param[in] fw_device the framework smp device that PHY CONTROL command
 *       targets to.
 * @param[in] phy_id The phy, whose route entry at route_index is to be configured.
 * @param[in] route_index The index of a phy's route entry that is to be configured.
 * @param[in] destination_sas_address A sas address for an route table entry
 *
 * @return void * address to the built scif sas smp request.
 */
void * scif_sas_smp_request_construct_config_route_info(
   struct SCIF_SAS_CONTROLLER    * fw_controller,
   struct SCIF_SAS_REMOTE_DEVICE * fw_device,
   U8                              phy_id,
   U16                             route_index,
   SCI_SAS_ADDRESS_T               destination_sas_address,
   BOOL                            disable_expander_route_entry
)
{
   SMP_REQUEST_T config_route_info;

   scif_sas_smp_protocol_request_construct(
      &config_route_info,
      SMP_FUNCTION_CONFIGURE_ROUTE_INFORMATION,
      0,
      sizeof(SMP_REQUEST_CONFIGURE_ROUTE_INFORMATION_T) / sizeof(U32)
   );

   config_route_info.request.configure_route_information.phy_identifier = phy_id;
   config_route_info.request.configure_route_information.expander_route_index_high =
      ((route_index & 0xff00) >> 8);
   config_route_info.request.configure_route_information.expander_route_index =
      route_index & 0xff;
   config_route_info.request.configure_route_information.routed_sas_address[0] =
      destination_sas_address.high;
   config_route_info.request.configure_route_information.routed_sas_address[1] =
      destination_sas_address.low;

   if (disable_expander_route_entry == TRUE)
      config_route_info.request.configure_route_information.disable_route_entry = 1;

   return scif_sas_smp_request_build(
             fw_controller, fw_device, &config_route_info,
             NULL, NULL
          );
}

/**
 * @brief This method retry the internal smp request.
 *
 * @param[in] fw_device This parameter specifies the remote device for
 *            which the internal IO request is destined.
 * @param[in] retry_count This parameter specifies how many times the
 *            old smp request has been retried.
 *
 * @return none.
 */
SCI_STATUS scif_sas_smp_internal_request_retry(
   SCIF_SAS_REMOTE_DEVICE_T * fw_device
)
{
   SCIF_SAS_CONTROLLER_T * fw_controller;
   SCIF_SAS_IO_REQUEST_T * new_io;
   void                  * new_request_memory = NULL;
   U8 retry_count = fw_device->protocol_device.smp_device.io_retry_count;

   SCIF_LOG_TRACE((
      sci_base_object_get_logger(fw_device),
      SCIF_LOG_OBJECT_IO_REQUEST | SCIF_LOG_OBJECT_DOMAIN_DISCOVERY,
      "scif_sas_smp_internal_request_retry(0x%x, 0x%x) time %d!\n",
      fw_device, retry_count
   ));

   fw_controller = fw_device->domain->controller;

   switch (fw_device->protocol_device.smp_device.current_smp_request)
   {
      case SMP_FUNCTION_REPORT_GENERAL:
         new_request_memory = scif_sas_smp_request_construct_report_general(
            fw_controller, fw_device
         );
         break;

      case SMP_FUNCTION_DISCOVER:
         //We are retrying an internal io. So we are going to allocate
         //a new memory from internal io memory pool.
         new_request_memory = scif_sas_smp_request_construct_discover(
            fw_controller, fw_device,
            fw_device->protocol_device.smp_device.current_activity_phy_index,
            NULL, NULL
         );

         break;

      case SMP_FUNCTION_REPORT_PHY_SATA:
         new_request_memory = scif_sas_smp_request_construct_report_phy_sata(
            fw_controller, fw_device,
            fw_device->protocol_device.smp_device.current_activity_phy_index
         );
         break;

      default:
         //unsupported case, TBD
         break;
   } //end of switch

   if (new_request_memory != NULL)
   {
      //set the retry count to new built smp request.
      new_io = (SCIF_SAS_IO_REQUEST_T *) new_request_memory;
      new_io->retry_count = ++retry_count;

      //need to schedule the DPC here.
      scif_cb_start_internal_io_task_schedule(
            fw_controller,
            scif_sas_controller_start_high_priority_io,
            fw_controller
         );

      return SCI_SUCCESS;
   }
   else
      return SCI_FAILURE_INSUFFICIENT_RESOURCES;

}

/**
 * @brief This method retry the external smp request.
 *
 * @param[in] fw_device This parameter specifies the remote device for
 *            which the internal IO request is destined.
 * @param[in] old_internal_io This parameter specifies the old smp request to be
 *            retried.
 *
 * @return none.
 */
SCI_STATUS scif_sas_smp_external_request_retry(
   SCIF_SAS_IO_REQUEST_T    * old_io
)
{
   SCIF_SAS_REMOTE_DEVICE_T * fw_device = old_io->parent.device;
   SCIF_SAS_CONTROLLER_T * fw_controller;
   SCIF_SAS_IO_REQUEST_T * new_io;
   void                  * new_request_memory = NULL;
   U8                      retry_count = old_io->retry_count;

   SCIF_LOG_TRACE((
      sci_base_object_get_logger(fw_device),
      SCIF_LOG_OBJECT_IO_REQUEST | SCIF_LOG_OBJECT_DOMAIN_DISCOVERY,
      "scif_sas_smp_external_request_retry(0x%x) time %d!\n",
      old_io
   ));

   fw_controller = fw_device->domain->controller;

   // Before we construct new io using the same memory, we need to
   // remove the IO from the list of outstanding requests on the domain
   // so that we don't damage the domain's fast list of request.
   sci_fast_list_remove_element(&old_io->parent.list_element);

   switch (fw_device->protocol_device.smp_device.current_smp_request)
   {
      case SMP_FUNCTION_DISCOVER:
         //we are retrying an external io, we are going to reuse the
         //old io's memory. new_request_memory is same as old_io.
         new_request_memory = scif_sas_smp_request_construct_discover(
            fw_controller, fw_device,
            fw_device->protocol_device.smp_device.current_activity_phy_index,
            (void *)sci_object_get_association(old_io),
            (void *)old_io
         );

         break;

      case SMP_FUNCTION_PHY_CONTROL:
         //Phy Control command always uses external io memory.
         new_request_memory = scif_sas_smp_request_construct_phy_control(
            fw_controller, fw_device, PHY_OPERATION_HARD_RESET,
            fw_device->protocol_device.smp_device.current_activity_phy_index,
            (void *)sci_object_get_association(old_io),
            (void *)old_io
         );

         break;

      default:
         //unsupported case, TBD
         return SCI_FAILURE;
   } //end of switch

   //set the retry count to new built smp request.
   new_io = (SCIF_SAS_IO_REQUEST_T *) new_request_memory;
   new_io->retry_count = ++retry_count;

   //put into the high priority queue.
   sci_pool_put(fw_controller->hprq.pool, (POINTER_UINT) new_request_memory);

   //schedule the DPC to start new io.
   scif_cb_start_internal_io_task_schedule(
      fw_controller, scif_sas_controller_start_high_priority_io, fw_controller
   );

   return SCI_SUCCESS;
}