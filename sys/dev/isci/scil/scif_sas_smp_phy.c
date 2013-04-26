
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
 * @brief This file contains the implementation of the SCIF_SAS_SMP_PHY
 *        object.
 */

#include <dev/isci/scil/scif_sas_controller.h>
#include <dev/isci/scil/scif_sas_smp_phy.h>
#include <dev/isci/scil/scif_sas_smp_remote_device.h>

//******************************************************************************
//*
//*     P U B L I C   M E T H O D S
//*
//******************************************************************************

/**
 * @brief This routine constructs a smp phy object for an expander phy and insert
 *           to owning expander device's smp_phy_list.
 * @param[in] this_smp_phy The memory space to store a phy
 * @param[in] owning_device The smp remote device that owns this smp phy.
 * @param[in] expander_phy_id The expander phy id for this_smp_phy.
 * @return None
 */
void scif_sas_smp_phy_construct(
   SCIF_SAS_SMP_PHY_T       * this_smp_phy,
   SCIF_SAS_REMOTE_DEVICE_T * owning_device,
   U8                         expander_phy_id
)
{
   memset(this_smp_phy, 0, sizeof(SCIF_SAS_SMP_PHY_T));

   this_smp_phy->phy_identifier = expander_phy_id;
   this_smp_phy->owning_device = owning_device;

   sci_fast_list_element_init((this_smp_phy), (&this_smp_phy->list_element));

   //insert to owning device's smp phy list.
   sci_fast_list_insert_tail(
      (&owning_device->protocol_device.smp_device.smp_phy_list),
      (&this_smp_phy->list_element)
   );
}

/**
 * @brief This routine destructs a smp phy object for an expander phy and free the smp
 *           phy to controller's smp phy memory.
 * @param[in] this_smp_phy The smp phy to be destructed.
 *
 * @return None
 */
void scif_sas_smp_phy_destruct(
   SCIF_SAS_SMP_PHY_T       * this_smp_phy
)
{
   SCIF_SAS_REMOTE_DEVICE_T * owning_device = this_smp_phy->owning_device;
   SCIF_SAS_CONTROLLER_T * fw_controller = owning_device->domain->controller;

   if ( ( this_smp_phy->attached_device_type == SMP_EDGE_EXPANDER_DEVICE
         || this_smp_phy->attached_device_type == SMP_FANOUT_EXPANDER_DEVICE)
       && this_smp_phy->u.attached_phy != NULL )
   {
      //update the counterpart phy from the other smp phy list.
      this_smp_phy->u.attached_phy->attached_device_type = SMP_NO_DEVICE_ATTACHED;
      this_smp_phy->u.attached_phy->u.attached_phy = NULL;
   }

   //remove curr_smp_phy
   sci_fast_list_remove_element(&this_smp_phy->list_element);
   scif_sas_controller_free_smp_phy(fw_controller, this_smp_phy);
}


/**
 * @brief This routine save a smp phy information based on discover response.
 *
 * @param[in] this_smp_phy The memory space to store a phy
 * @param[in] attached_device A possible direct attached device to this phy.
 *
 * @param[in] discover_response The smp DISCOVER response for this_smp_phy.
 * @return None
 */
void scif_sas_smp_phy_save_information(
   SCIF_SAS_SMP_PHY_T       * this_smp_phy,
   SCIF_SAS_REMOTE_DEVICE_T * attached_device,
   SMP_RESPONSE_DISCOVER_T  * discover_response
)
{
   ASSERT (this_smp_phy->owning_device != NULL);
   ASSERT (this_smp_phy->phy_identifier == discover_response->phy_identifier);

   this_smp_phy->attached_device_type = (U8)discover_response->u2.sas1_1.attached_device_type;
   this_smp_phy->routing_attribute = (U8)discover_response->routing_attribute;
   this_smp_phy->attached_sas_address = discover_response->attached_sas_address;
   this_smp_phy->config_route_table_index_anchor = 0;

   if (this_smp_phy->attached_device_type != SMP_EDGE_EXPANDER_DEVICE
       && this_smp_phy->attached_device_type != SMP_FANOUT_EXPANDER_DEVICE)
   {
      //note, end_device field could be an end device, or a NULL value, but can't be expander device.
      this_smp_phy->u.end_device = attached_device;
   }
   else
   {
      //if attached device type is expander, we will set u.attached_phy later when the
      //the attached expander finish its discover on attached_phy.
      ;
   }
}

/**
 * @brief This routine constructs a smp phy object for an expander phy.
 * @param[in] this_smp_phy The memory space to store a phy
 * @param[in] owning_device The smp remote device that owns this smp phy.
 * @param[in] discover_response The smp DISCOVER response for this_smp_phy.
 *
 * @return Whether a smp phy has an attached phy and the pair of phy are set
 *            set to each other as attached phy successfully.
 */
SCI_STATUS scif_sas_smp_phy_set_attached_phy(
   SCIF_SAS_SMP_PHY_T       * this_smp_phy,
   U8                         attached_phy_identifier,
   SCIF_SAS_REMOTE_DEVICE_T * attached_remote_device
)
{
   //find the attached phy from its owning device by attached_phy_id.
   SCIF_SAS_SMP_PHY_T * attached_smp_phy =
      (SCIF_SAS_SMP_PHY_T *)scif_sas_smp_remote_device_find_smp_phy_by_id(
          attached_phy_identifier,
          &attached_remote_device->protocol_device.smp_device);

   if (attached_smp_phy != NULL)
   {
      this_smp_phy->u.attached_phy = attached_smp_phy;
      attached_smp_phy->u.attached_phy = this_smp_phy;

      return SCI_SUCCESS;
   }

   return SCI_FAILURE;
}


/**
 * @brief This method verify the routing attributes of a phy connection per
 *        specification.
 *
 * @param[in] this_smp_phy One smp phy belongs to a smp phy connection.
 * @param[in] attached_smp_phy One smp phy belongs to a smp phy connection.
 *
 * @return Whether routing attributes of a phy connection is legal.
 * @retval SCI_SUCCESS indicates a good phy connection.
 *         SCI_FAILURE indicates a illegal phy connection.
 */
SCI_STATUS scif_sas_smp_phy_verify_routing_attribute(
   SCIF_SAS_SMP_PHY_T * this_smp_phy,
   SCIF_SAS_SMP_PHY_T * attached_smp_phy
)
{
   SCI_STATUS status = SCI_SUCCESS;

   //expander phy with direct routing attribute can only connect to
   //phy with direct routing attribute.
   if ( this_smp_phy->routing_attribute == DIRECT_ROUTING_ATTRIBUTE
         || attached_smp_phy->routing_attribute == DIRECT_ROUTING_ATTRIBUTE )
   {
      if ( (this_smp_phy->routing_attribute | attached_smp_phy->routing_attribute)
           != DIRECT_ROUTING_ATTRIBUTE )
         status = SCI_FAILURE;
   }

   if (this_smp_phy->routing_attribute == TABLE_ROUTING_ATTRIBUTE
       && attached_smp_phy->routing_attribute == TABLE_ROUTING_ATTRIBUTE)
   {
      if ( ! this_smp_phy->owning_device->protocol_device.smp_device.is_table_to_table_supported
         || !attached_smp_phy->owning_device->protocol_device.smp_device.is_table_to_table_supported )
      status = SCI_FAILURE;
   }

   return status;
}


/**
 * @brief This method find The next smp phy that is in the smp phy list and
 *           resides in the same wide port as this_smp_phy.
 *
 * @param[in] this_smp_phy The smp phy whose neighbor phy that is in the same
 *               same wide port is to be find.
 *
 * @return The next smp phy that is in the smp phy list and resides in the same
 *            wide port as this_smp_phy.
 */
SCIF_SAS_SMP_PHY_T * scif_sas_smp_phy_find_next_phy_in_wide_port(
   SCIF_SAS_SMP_PHY_T * this_smp_phy
)
{
   SCI_FAST_LIST_ELEMENT_T * next_phy_element = sci_fast_list_get_next(
      &(this_smp_phy->list_element) );

   SCIF_SAS_SMP_PHY_T * next_phy;

   while (next_phy_element != NULL)
   {
      next_phy = (SCIF_SAS_SMP_PHY_T *)sci_fast_list_get_object(next_phy_element);

      next_phy_element = sci_fast_list_get_next( &(next_phy->list_element));

      if (next_phy->attached_sas_address.high == this_smp_phy->attached_sas_address.high
          &&next_phy->attached_sas_address.low == this_smp_phy->attached_sas_address.low)
         return next_phy;
   }

   return NULL;
}


/**
 * @brief This method find the smp phy that resides in the middle of the same
 *        wide port as this_smp_phy.
 *
 * @param[in] this_smp_phy The smp phy who is the lowest order phy in a wide
 *       port .
 *
 * @return The next smp phy that is in the smp phy list and resides in the same
 *            wide port as this_smp_phy.
 */
SCIF_SAS_SMP_PHY_T * scif_sas_smp_phy_find_middle_phy_in_wide_port(
   SCIF_SAS_SMP_PHY_T * this_smp_phy
)
{
   SCIF_SAS_SMP_PHY_T * next_phy =
      scif_sas_smp_phy_find_next_phy_in_wide_port(this_smp_phy);
   SCIF_SAS_SMP_PHY_T * middle_phy = this_smp_phy;

   //currently we assume a wide port could not be wider than X4. so the
   //second phy is always the correct answer for x2, x3 or x4 wide port.
   //For a narrow port, phy0 is the middle phy.
   if (next_phy != NULL)
   {
      middle_phy = next_phy;
      next_phy =
         scif_sas_smp_phy_find_next_phy_in_wide_port(next_phy);
   }

   if (next_phy != NULL)
      middle_phy = next_phy;

   return middle_phy;
}


/**
 * @brief This method find the smp phy that is the hishest order phy
 *           in the same wide port as this_smp_phy.
 *
 * @param[in] this_smp_phy The smp phy who is the lowest order phy in a wide
 *       port.
 *
 * @return The next smp phy that is in the smp phy list and resides in the same
 *            wide port as this_smp_phy.
 */
SCIF_SAS_SMP_PHY_T * scif_sas_smp_phy_find_highest_phy_in_wide_port(
   SCIF_SAS_SMP_PHY_T * this_smp_phy
)
{
   SCIF_SAS_SMP_PHY_T * next_phy =
      scif_sas_smp_phy_find_next_phy_in_wide_port(this_smp_phy);
   SCIF_SAS_SMP_PHY_T * highest_phy = this_smp_phy;

   while(next_phy != NULL )
   {
      highest_phy = next_phy;
      next_phy =
         scif_sas_smp_phy_find_next_phy_in_wide_port(next_phy);
   }

   return highest_phy;
}