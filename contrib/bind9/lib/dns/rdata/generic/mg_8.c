
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

/* $Id: mg_8.c,v 1.45 2009/12/04 22:06:37 tbox Exp $ */

/* reviewed: Wed Mar 15 17:49:21 PST 2000 by brister */

#ifndef RDATA_GENERIC_MG_8_C
#define RDATA_GENERIC_MG_8_C

#define RRTYPE_MG_ATTRIBUTES (0)

static inline isc_result_t
fromtext_mg(ARGS_FROMTEXT) {
	isc_token_t token;
	dns_name_t name;
	isc_buffer_t buffer;

	REQUIRE(type == 8);

	UNUSED(type);
	UNUSED(rdclass);
	UNUSED(callbacks);

	RETERR(isc_lex_getmastertoken(lexer, &token, isc_tokentype_string,
				      ISC_FALSE));

	dns_name_init(&name, NULL);
	buffer_fromregion(&buffer, &token.value.as_region);
	origin = (origin != NULL) ? origin : dns_rootname;
	RETTOK(dns_name_fromtext(&name, &buffer, origin, options, target));
	return (ISC_R_SUCCESS);
}

static inline isc_result_t
totext_mg(ARGS_TOTEXT) {
	isc_region_t region;
	dns_name_t name;
	dns_name_t prefix;
	isc_boolean_t sub;

	REQUIRE(rdata->type == 8);
	REQUIRE(rdata->length != 0);

	dns_name_init(&name, NULL);
	dns_name_init(&prefix, NULL);

	dns_rdata_toregion(rdata, &region);
	dns_name_fromregion(&name, &region);

	sub = name_prefix(&name, tctx->origin, &prefix);

	return (dns_name_totext(&prefix, sub, target));
}

static inline isc_result_t
fromwire_mg(ARGS_FROMWIRE) {
	dns_name_t name;

	REQUIRE(type == 8);

	UNUSED(type);
	UNUSED(rdclass);

	dns_decompress_setmethods(dctx, DNS_COMPRESS_GLOBAL14);

	dns_name_init(&name, NULL);
	return (dns_name_fromwire(&name, source, dctx, options, target));
}

static inline isc_result_t
towire_mg(ARGS_TOWIRE) {
	dns_name_t name;
	dns_offsets_t offsets;
	isc_region_t region;

	REQUIRE(rdata->type == 8);
	REQUIRE(rdata->length != 0);

	dns_compress_setmethods(cctx, DNS_COMPRESS_GLOBAL14);

	dns_name_init(&name, offsets);
	dns_rdata_toregion(rdata, &region);
	dns_name_fromregion(&name, &region);

	return (dns_name_towire(&name, cctx, target));
}

static inline int
compare_mg(ARGS_COMPARE) {
	dns_name_t name1;
	dns_name_t name2;
	isc_region_t region1;
	isc_region_t region2;

	REQUIRE(rdata1->type == rdata2->type);
	REQUIRE(rdata1->rdclass == rdata2->rdclass);
	REQUIRE(rdata1->type == 8);
	REQUIRE(rdata1->length != 0);
	REQUIRE(rdata2->length != 0);

	dns_name_init(&name1, NULL);
	dns_name_init(&name2, NULL);

	dns_rdata_toregion(rdata1, &region1);
	dns_rdata_toregion(rdata2, &region2);

	dns_name_fromregion(&name1, &region1);
	dns_name_fromregion(&name2, &region2);

	return (dns_name_rdatacompare(&name1, &name2));
}

static inline isc_result_t
fromstruct_mg(ARGS_FROMSTRUCT) {
	dns_rdata_mg_t *mg = source;
	isc_region_t region;

	REQUIRE(type == 8);
	REQUIRE(source != NULL);
	REQUIRE(mg->common.rdtype == type);
	REQUIRE(mg->common.rdclass == rdclass);

	UNUSED(type);
	UNUSED(rdclass);

	dns_name_toregion(&mg->mg, &region);
	return (isc_buffer_copyregion(target, &region));
}

static inline isc_result_t
tostruct_mg(ARGS_TOSTRUCT) {
	isc_region_t region;
	dns_rdata_mg_t *mg = target;
	dns_name_t name;

	REQUIRE(rdata->type == 8);
	REQUIRE(target != NULL);
	REQUIRE(rdata->length != 0);

	mg->common.rdclass = rdata->rdclass;
	mg->common.rdtype = rdata->type;
	ISC_LINK_INIT(&mg->common, link);

	dns_name_init(&name, NULL);
	dns_rdata_toregion(rdata, &region);
	dns_name_fromregion(&name, &region);
	dns_name_init(&mg->mg, NULL);
	RETERR(name_duporclone(&name, mctx, &mg->mg));
	mg->mctx = mctx;
	return (ISC_R_SUCCESS);
}

static inline void
freestruct_mg(ARGS_FREESTRUCT) {
	dns_rdata_mg_t *mg = source;

	REQUIRE(source != NULL);
	REQUIRE(mg->common.rdtype == 8);

	if (mg->mctx == NULL)
		return;
	dns_name_free(&mg->mg, mg->mctx);
	mg->mctx = NULL;
}

static inline isc_result_t
additionaldata_mg(ARGS_ADDLDATA) {
	REQUIRE(rdata->type == 8);

	UNUSED(add);
	UNUSED(arg);
	UNUSED(rdata);

	return (ISC_R_SUCCESS);
}

static inline isc_result_t
digest_mg(ARGS_DIGEST) {
	isc_region_t r;
	dns_name_t name;

	REQUIRE(rdata->type == 8);

	dns_rdata_toregion(rdata, &r);
	dns_name_init(&name, NULL);
	dns_name_fromregion(&name, &r);

	return (dns_name_digest(&name, digest, arg));
}

static inline isc_boolean_t
checkowner_mg(ARGS_CHECKOWNER) {

	REQUIRE(type == 8);

	UNUSED(type);
	UNUSED(rdclass);
	UNUSED(wildcard);

	return (dns_name_ismailbox(name));
}

static inline isc_boolean_t
checknames_mg(ARGS_CHECKNAMES) {

	REQUIRE(rdata->type == 8);

	UNUSED(rdata);
	UNUSED(owner);
	UNUSED(bad);

	return (ISC_TRUE);
}

static inline int
casecompare_mg(ARGS_COMPARE) {
	return (compare_mg(rdata1, rdata2));
}

#endif	/* RDATA_GENERIC_MG_8_C */