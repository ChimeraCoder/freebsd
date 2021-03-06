
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

/*! @mainpage Heimdal GSS-API Library
 *
 * Heimdal implements the following mechanisms:
 *
 * - Kerberos 5
 * - SPNEGO
 * - NTLM
 *
 * See @ref gssapi_mechs for more describtion about these mechanisms.
 *
 * The project web page: http://www.h5l.org/
 *
 * - @ref gssapi_services_intro
 * - @ref gssapi_mechs
 * - @ref gssapi_api_INvsMN
 */

/**
 * @page gssapi_services_intro Introduction to GSS-API services
 * @section gssapi_services GSS-API services
 *
 * @subsection gssapi_services_context Context creation
 *
 *  - delegation
 *  - mutual authentication
 *  - anonymous
 *  - use per message before context creation has completed
 *
 *  return status:
 *  - support conf
 *  - support int
 *
 * @subsection gssapi_context_flags Context creation flags
 *
 * - GSS_C_DELEG_FLAG
 * - GSS_C_MUTUAL_FLAG
 * - GSS_C_REPLAY_FLAG
 * - GSS_C_SEQUENCE_FLAG
 * - GSS_C_CONF_FLAG
 * - GSS_C_INTEG_FLAG
 * - GSS_C_ANON_FLAG
 * - GSS_C_PROT_READY_FLAG
 * - GSS_C_TRANS_FLAG
 * - GSS_C_DCE_STYLE
 * - GSS_C_IDENTIFY_FLAG
 * - GSS_C_EXTENDED_ERROR_FLAG
 * - GSS_C_DELEG_POLICY_FLAG
 *
 *
 * @subsection gssapi_services_permessage Per-message services
 *
 *  - conf
 *  - int
 *  - message integrity
 *  - replay detection
 *  - out of sequence
 *
 */

/**
 * @page gssapi_mechs_intro GSS-API mechanisms
 * @section gssapi_mechs GSS-API mechanisms
 *
 * - Kerberos 5 - GSS_KRB5_MECHANISM
 * - SPNEGO - GSS_SPNEGO_MECHANISM
 * - NTLM - GSS_NTLM_MECHANISM

 */


/**
 * @page internalVSmechname Internal names and mechanism names
 * @section gssapi_api_INvsMN Name forms
 *
 * There are two forms of name in GSS-API, Internal form and
 * Contiguous string ("flat") form. gss_export_name() and
 * gss_import_name() can be used to convert between the two forms.
 *
 * - The contiguous string form is described by an oid specificing the
 *   type and an octet string. A special form of the contiguous
 *   string form is the exported name object. The exported name
 *   defined for each mechanism, is something that can be stored and
 *   complared later. The exported name is what should be used for
 *   ACLs comparisons.
 *
 * - The Internal form
 *
 *   There is also special form of the Internal Name (IN), and that is
 *   the Mechanism Name (MN). In the mechanism name all the generic
 *   information is stripped of and only contain the information for
 *   one mechanism.  In GSS-API some function return MN and some
 *   require MN as input. Each of these function is marked up as such.
 *
 *
 * Describe relationship between import_name, canonicalize_name,
 * export_name and friends.
 */

/** @defgroup gssapi Heimdal GSS-API functions */