
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

/*
 * Decode a UNI message header.
 * Return values:
 *   0 - ok
 *  -1 - ignore message (proto, length, CR error)
 */
int
uni_decode_head(struct uni_msg *msg, struct uni_all *out,
    struct unicx *cx __unused)
{
	u_int mlen;

	cx->errcnt = 0;
	(void)memset(out, 0, sizeof(struct uni_all));

	if(uni_msg_len(msg) < 9)
		return -1;			/* Q.2931 5.6.2 */
	if(cx->pnni) {
		if(*msg->b_rptr++ != PNNI_PROTO)
			return -1;			/* Q.2931 5.6.1 */
	} else {
		if(*msg->b_rptr++ != UNI_PROTO)
			return -1;			/* Q.2931 5.6.1 */
	}
	if(*msg->b_rptr++ != 3)
		return -1;			/* Q.2931 5.6.3.1 */

	out->u.hdr.cref.flag = (*msg->b_rptr & 0x80) ? 1 : 0;
	out->u.hdr.cref.cref = (*msg->b_rptr++ & 0x7f) << 16;
	out->u.hdr.cref.cref |= *msg->b_rptr++ << 8;
	out->u.hdr.cref.cref |= *msg->b_rptr++;

	out->mtype = *msg->b_rptr++;

	/*
	 * Be not too piggy about this byte
	 */
	switch(*msg->b_rptr & 0x13) {

	  case 0x00: case 0x01: case 0x02: case 0x03:
		out->u.hdr.act = UNI_MSGACT_DEFAULT;
		break;

	  case 0x10: case 0x11: case 0x12:
		out->u.hdr.act = *msg->b_rptr & 0x3;
		break;

	  case 0x13:			/* Q.2931 5.7.1 */
		out->u.hdr.act = UNI_MSGACT_REPORT;
		break;
	}
	if(cx->pnni && (*msg->b_rptr & 0x08))
		out->u.hdr.pass = 1;
	else
		out->u.hdr.pass = 0;
	
	msg->b_rptr++;

	mlen = *msg->b_rptr++ << 8;
	mlen |= *msg->b_rptr++;

	/*
	 * If the message is longer than the indicated length
	 * shorten it. If it is shorter, probably one of the IE
	 * decoders will break, but we should proceed. 5.5.6.5
	 */
#if 0
	if(uni_msg_len(msg) > mlen)
		msg->b_wptr = msg->b_rptr + mlen;
#endif

	return 0;
}

static int
uni_decode_body_internal(enum uni_msgtype mtype, struct uni_msg *msg,
    union uni_msgall *out, struct unicx *cx)
{
	enum uni_ietype ietype;
	struct uni_iehdr hdr;
	u_int ielen;
	const struct iedecl *iedecl;
	int err = 0, ret;
	u_char *end;

	cx->ielast = (enum uni_ietype)0;
	cx->repeat.h.present = 0;

	while (uni_msg_len(msg) != 0) {
		if (uni_decode_ie_hdr(&ietype, &hdr, msg, cx, &ielen)) {
			/*
			 * Short header. Set the ielen to an impossible size.
			 * Then we should bump out in the error handling below.
			 * We should have at least an IE type here.
			 */
			ielen = 0xffffffff;
		}
#ifdef DTRACE
		printf("IE %x\n", ietype);
#endif

		if ((iedecl = GET_IEDECL(ietype, hdr.coding)) == NULL ||
		    ietype == UNI_IE_UNREC) {
			/*
			 * entirly unknown IE. Check the length and skip it.
			 * Q.2931 5.6.8.1
			 */
			if (ielen > uni_msg_len(msg))
				msg->b_rptr = msg->b_wptr;
			else
				msg->b_rptr += ielen;
			(void)UNI_SAVE_IERR(cx, ietype, hdr.act, UNI_IERR_UNK);
			err = -1;
			continue;
		}
#ifdef DTRACE
		printf("IE %x known\n", ietype);
#endif
		if (ielen > iedecl->maxlen - 4 || ielen > uni_msg_len(msg)) {
			/*
			 * Information element too long -> content error.
			 * Let the decoding routine set the error flag and
			 * return DEC_ERR.
			 * Q.2931 5.6.8.2
			 */
#if 0
			/*
			 * It is not clear how to best handle this error.
			 */
			if (ielen > iedecl->maxlen - 4)
				ielen = iedecl->maxlen - 4;
#endif

			if (ielen > uni_msg_len(msg))
				ielen = uni_msg_len(msg);

			hdr.present |= UNI_IE_ERROR;

#ifdef DTRACE
			printf("IE %x length too large\n", ietype);
#endif
		}

#ifdef DTRACE
		else
			printf("IE %x length ok\n", ietype);
#endif
		end = msg->b_rptr + ielen;
		ret = uni_msgtable[mtype]->decode(out, msg, ietype,
		    &hdr, ielen, cx);
		msg->b_rptr = end;

#ifdef DTRACE
		printf("IE %x ret %d\n", ietype, ret);
#endif

		switch (ret) {

		  case DEC_OK:	/* ok */
			break;

		  case DEC_ILL:	/* illegal IE */
			/*
			 * Unexpected but recognized.
			 * Q.2931 5.6.8.3
			 */
			(void)UNI_SAVE_IERR(cx, ietype, hdr.act, UNI_IERR_UNK);
			err = -1;
			break;

		  case DEC_ERR:	/* bad IE */
			if (iedecl->flags & UNIFL_ACCESS)
				/* this may be wrong: 5.6.8.2 */
				(void)UNI_SAVE_IERR(cx, ietype, hdr.act, UNI_IERR_ACC);
			else
				(void)UNI_SAVE_IERR(cx, ietype, hdr.act, UNI_IERR_BAD);
			err = -1;
			break;

		  default:
			PANIC(("bad decode return"));
		}
	    	cx->ielast = ietype;
	    	if (ietype != UNI_IE_REPEAT)
			cx->repeat.h.present = 0;
	}
	return err;
}

/*
 * Decode the body of a message. The header is assumed to be decoded
 * already and out->hdr is filled in. Only information elements remain.
 */
int
uni_decode_body(struct uni_msg *msg, struct uni_all *out, struct unicx *cx)
{
	cx->errcnt = 0;
	if (out->mtype >= 256)
		return (-1);
	if (uni_msgtable[out->mtype] == NULL)
		return (-1);
	return (uni_decode_body_internal(out->mtype, msg, &out->u, cx));
}


/*
 * Decode a uni message
 */
int
uni_decode(struct uni_msg *msg, struct uni_all *out, struct unicx *cx)
{
	cx->errcnt = 0;
	if (uni_decode_head(msg, out, cx))
		return (-1);
	if (uni_decode_body(msg, out, cx))
		return (-2);
	return (0);
}

int
uni_encode(struct uni_msg *msg, struct uni_all *in, struct unicx *cx)
{
	if (in->mtype >= 256)
		return (-1);
	if (uni_msgtable[in->mtype] == NULL)
		return (-3);

	return ((uni_msgtable[in->mtype]->encode)(msg, &in->u, cx));
}

/*
 * Doesn't belong here
 */
void
uni_initcx(struct unicx *cx)
{
	memset(cx, 0, sizeof(struct unicx));
	cx->tabsiz = 4;
}