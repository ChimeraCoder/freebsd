
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

#include <sendmail.h>

SM_RCSID("@(#)$Id: sysexits.c,v 8.34 2002/09/09 02:43:00 gshapiro Exp $")

/*
**  DSNTOEXITSTAT -- convert DSN-style error code to EX_ style.
**
**	Parameters:
**		dsncode -- the text of the DSN-style code.
**
**	Returns:
**		The corresponding exit status.
*/

int
dsntoexitstat(dsncode)
	char *dsncode;
{
	int code2, code3;

	/* first the easy cases.... */
	if (*dsncode == '2')
		return EX_OK;
	if (*dsncode == '4')
		return EX_TEMPFAIL;

	/* reject other illegal values */
	if (*dsncode != '5')
		return EX_CONFIG;

	/* now decode the other two field parts */
	if (*++dsncode == '.')
		dsncode++;
	code2 = atoi(dsncode);
	while (*dsncode != '\0' && *dsncode != '.')
		dsncode++;
	if (*dsncode != '\0')
		dsncode++;
	code3 = atoi(dsncode);

	/* and do a nested switch to work them out */
	switch (code2)
	{
	  case 0:	/* Other or Undefined status */
		return EX_UNAVAILABLE;

	  case 1:	/* Address Status */
		switch (code3)
		{
		  case 0:	/* Other Address Status */
			return EX_DATAERR;

		  case 1:	/* Bad destination mailbox address */
		  case 6:	/* Mailbox has moved, No forwarding address */
			return EX_NOUSER;

		  case 2:	/* Bad destination system address */
		  case 8:	/* Bad senders system address */
			return EX_NOHOST;

		  case 3:	/* Bad destination mailbox address syntax */
		  case 7:	/* Bad senders mailbox address syntax */
			return EX_USAGE;

		  case 4:	/* Destination mailbox address ambiguous */
			return EX_UNAVAILABLE;

		  case 5:	/* Destination address valid */
			/* According to RFC1893, this can't happen */
			return EX_CONFIG;
		}
		break;

	  case 2:	/* Mailbox Status */
		switch (code3)
		{
		  case 0:	/* Other or Undefined mailbox status */
		  case 1:	/* Mailbox disabled, not accepting messages */
		  case 2:	/* Mailbox full */
		  case 4:	/* Mailing list expansion problem */
			return EX_UNAVAILABLE;

		  case 3:	/* Message length exceeds administrative lim */
			return EX_DATAERR;
		}
		break;

	  case 3:	/* System Status */
		return EX_OSERR;

	  case 4:	/* Network and Routing Status */
		switch (code3)
		{
		  case 0:	/* Other or undefined network or routing stat */
			return EX_IOERR;

		  case 1:	/* No answer from host */
		  case 3:	/* Routing server failure */
		  case 5:	/* Network congestion */
			return EX_TEMPFAIL;

		  case 2:	/* Bad connection */
			return EX_IOERR;

		  case 4:	/* Unable to route */
			return EX_PROTOCOL;

		  case 6:	/* Routing loop detected */
			return EX_CONFIG;

		  case 7:	/* Delivery time expired */
			return EX_UNAVAILABLE;
		}
		break;

	  case 5:	/* Protocol Status */
		return EX_PROTOCOL;

	  case 6:	/* Message Content or Media Status */
		return EX_UNAVAILABLE;

	  case 7:	/* Security Status */
		return EX_DATAERR;
	}
	return EX_UNAVAILABLE;
}
/*
**  EXITSTAT -- convert EX_ value to error text.
**
**	Parameters:
**		excode -- rstatus which might consists of an EX_* value.
**
**	Returns:
**		The corresponding error text or the original string.
*/

char *
exitstat(excode)
	char *excode;
{
	char *c;
	int i;
	char *exitmsg;

	if (excode == NULL || *excode == '\0')
		return excode;
	i = (int) strtol(excode, &c, 10);
	if (*c != '\0')
		return excode;
	exitmsg = sm_sysexitmsg(i);
	if (exitmsg != NULL)
		return exitmsg;
	return excode;
}