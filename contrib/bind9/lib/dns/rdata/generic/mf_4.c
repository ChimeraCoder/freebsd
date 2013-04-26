
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

/* $Id: mf_4.c,v 1.47 2009/12/04 22:06:37 tbox Exp $ */

/* reviewed: Wed Mar 15 17:47:33 PST 2000 by brister */

#ifndef RDATA_GENERIC_MF_4_C
#define RDATA_GENERIC_MF_4_C

#define RRTYPE_MF_ATTRIBUTES (0)

static inline isc_result_t
fromtext_mf(ARGS_FROMTEXT) {
	isc_token_t token;
	dns_name_t name;
	isc_buffer_t buffer;

	REQUIRE(type == 4);

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
totext_mf(ARGS_TOTEXT) {
	isc_region_t region;
	dns_name_t name;
	dns_name_t prefix;
	isc_boolean_t sub;

	REQUIRE(rdata->type == 4);
	REQUIRE(rdata->length != 0);

	dns_name_init(&name, NULL);
	dns_name_init(&prefix, NULL);

	dns_rdata_toregion(rdata, &region);
	dns_name_fromregion(&name, &region);

	sub = name_prefix(&name, tctx->origin, &prefix);

	return (dns_name_totext(&prefix, sub, target));
}

static inline isc_result_t
fromwire_mf(ARGS_FROMWIRE) {
	dns_name_t name;

	REQUIRE(type == 4);

	UNUSED(type);
	UNUSED(rdclass);

	dns_decompress_setmethods(dctx, DNS_COMPRESS_GLOBAL14);

	dns_name_init(&name, NULL);
	return (dns_name_fromwire(&name, source, dctx, options, target));
}

static inline isc_result_t
towire_mf(ARGS_TOWIRE) {
	dns_name_t name;
	dns_offsets_t offsets;
	isc_region_t region;

	REQUIRE(rdata->type == 4);
	REQUIRE(rdata->length != 0);

	dns_compress_setmethods(cctx, DNS_COMPRESS_GLOBAL14);

	dns_name_init(&name, offsets);
	dns_rdata_toregion(rdata, &region);
	dns_name_fromregion(&name, &region);

	return (dns_name_towire(&name, cctx, target));
}

static inline int
compare_mf(ARGS_COMPARE) {
	dns_name_t name1;
	dns_name_t name2;
	isc_region_t region1;
	isc_region_t region2;

	REQUIRE(rdata1->type == rdata2->type);
	REQUIRE(rdata1->rdclass == rdata2->rdclass);
	REQUIRE(rdata1->type == 4);
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
fromstruct_mf(ARGS_FROMSTRUCT) {
	dns_rdata_mf_t *mf = source;
	isc_region_t region;

	REQUIRE(type == 4);
	REQUIRE(source != NULL);
	REQUIRE(mf->common.rdtype == type);
	REQUIRE(mf->common.rdclass == rdclass);

	UNUSED(type);
	UNUSED(rdclass);

	dns_name_toregion(&mf->mf, &region);
	return (isc_buffer_copyregion(target, &region));
}

static inline isc_result_t
tostruct_mf(ARGS_TOSTRUCT) {
	dns_rdata_mf_t *mf = target;
	isc_region_t r;
	dns_name_t name;

	REQUIRE(rdata->type == 4);
	REQUIRE(target != NULL);
	REQUIRE(rdata->length != 0);

	mf->common.rdclass = rdata->rdclass;
	mf->common.rdtype = rdata->type;
	ISC_LINK_INIT(&mf->common, link);

	dns_name_init(&name, NULL);
	dns_rdata_toregion(rdata, &r);
	dns_name_fromregion(&name, &r);
	dns_name_init(&mf->mf, NULL);
	RETERR(name_duporclone(&name, mctx, &mf->mf));
	mf->mctx = mctx;
	return (ISC_R_SUCCESS);
}

static inline void
freestruct_mf(ARGS_FREESTRUCT) {
	dns_rdata_mf_t *mf = source;

	REQUIRE(source != NULL);
	REQUIRE(mf->common.rdtype == 4);

	if (mf->mctx == NULL)
		return;
	dns_name_free(&mf->mf, mf->mctx);
	mf->mctx = NULL;
}

static inline isc_result_t
additionaldata_mf(ARGS_ADDLDATA) {
	dns_name_t name;
	dns_offsets_t offsets;
	isc_region_t region;

	REQUIRE(rdata->type == 4);

	dns_name_init(&name, offsets);
	dns_rdata_toregion(rdata, &region);
	dns_name_fromregion(&name, &region);

	return ((add)(arg, &name, dns_rdatatype_a));
}

static inline isc_result_t
digest_mf(ARGS_DIGEST) {
	isc_region_t r;
	dns_name_t name;

	REQUIRE(rdata->type == 4);

	dns_rdata_toregion(rdata, &r);
	dns_name_init(&name, NULL);
	dns_name_fromregion(&name, &r);

	return (dns_name_digest(&name, digest, arg));
}

static inline isc_boolean_t
checkowner_mf(ARGS_CHECKOWNER) {

	REQUIRE(type == 4);

	UNUSED(name);
	UNUSED(type);
	UNUSED(rdclass);
	UNUSED(wildcard);

	return (ISC_TRUE);
}

static inline isc_boolean_t
checknames_mf(ARGS_CHECKNAMES) {

	REQUIRE(rdata->type == 4);

	UNUSED(rdata);
	UNUSED(owner);
	UNUSED(bad);

	return (ISC_TRUE);
}

static inline int
casecompare_mf(ARGS_COMPARE) {
	return (compare_mf(rdata1, rdata2));
}

#endif	/* RDATA_GENERIC_MF_4_C */