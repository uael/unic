/*
 * Copyright (C) 2013-2016 Alexander Saprykin <xelfium@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLIBSYS_TESTS_STATIC
#  define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE pcryptohash_test

#include "plib.h"

#include <string.h>

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

#define PCRYPTO_STRESS_LENGTH	10000
#define PCRYPTO_MAX_UPDATES	1000000

extern "C" ptr_t pmem_alloc (size_t nbytes)
{
	P_UNUSED (nbytes);
	return (ptr_t) NULL;
}

extern "C" ptr_t pmem_realloc (ptr_t block, size_t nbytes)
{
	P_UNUSED (block);
	P_UNUSED (nbytes);
	return (ptr_t) NULL;
}

extern "C" void pmem_free (ptr_t block)
{
	P_UNUSED (block);
}

static void
general_hash_test (hash_kind_t	type,
		   size_t		hash_len,
		   const byte_t		*msg1,
		   const byte_t		*msg2,
		   const ubyte_t		*etalon1,
		   const ubyte_t		*etalon2,
		   const ubyte_t		*etalon3,
		   const byte_t		*hash1,
		   const byte_t		*hash2,
		   const byte_t		*hash3,
		   const byte_t		*hash_stress)
{
	hash_t	*crypto_hash;
	size_t		dig_len;
	byte_t		*hash_str;
	byte_t		*long_str;
	ubyte_t		*hash_dig;

	crypto_hash = p_crypto_hash_new (type);

	BOOST_REQUIRE ((size_t) p_crypto_hash_get_length (crypto_hash) == hash_len);
	BOOST_REQUIRE (p_crypto_hash_get_type (crypto_hash) == type);

	hash_str = p_crypto_hash_get_string (crypto_hash);
	BOOST_REQUIRE (hash_str != NULL);
	p_crypto_hash_reset (crypto_hash);
	p_free (hash_str);

	hash_dig = (ubyte_t *) p_malloc0 (hash_len);
	BOOST_REQUIRE (hash_dig != NULL);

	long_str = (byte_t *) p_malloc0 (PCRYPTO_STRESS_LENGTH);
	BOOST_REQUIRE (long_str != NULL);

	for (int i = 0; i < PCRYPTO_STRESS_LENGTH; ++i)
		long_str[i] = (byte_t) (97 + i % 20);

	/* Case 1 */

	/* Check string */
	p_crypto_hash_update (crypto_hash, (const ubyte_t *) msg1, strlen (msg1));
	hash_str = p_crypto_hash_get_string (crypto_hash);
	BOOST_CHECK (strcmp (hash_str, hash1) == 0);
	p_free (hash_str);

	p_crypto_hash_reset (crypto_hash);

	/* Check digest */
	dig_len = hash_len;
	p_crypto_hash_update (crypto_hash, (const ubyte_t *) msg1, strlen (msg1));
	p_crypto_hash_get_digest (crypto_hash, hash_dig, &dig_len);

	BOOST_CHECK (dig_len == hash_len);

	for (unsigned int i = 0; i < hash_len; ++i)
		BOOST_CHECK (hash_dig[i] == etalon1[i]);

	p_crypto_hash_reset (crypto_hash);

	/* Case 2 */

	/* Check string */
	p_crypto_hash_update (crypto_hash, (const ubyte_t *) msg2, strlen (msg2));
	hash_str = p_crypto_hash_get_string (crypto_hash);
	BOOST_CHECK (strcmp (hash_str, hash2) == 0);
	p_free (hash_str);

	p_crypto_hash_reset (crypto_hash);

	/* Check digest */
	dig_len = hash_len;
	p_crypto_hash_update (crypto_hash, (const ubyte_t *) msg2, strlen (msg2));
	p_crypto_hash_get_digest (crypto_hash, hash_dig, &dig_len);

	BOOST_CHECK (dig_len == hash_len);

	for (unsigned int i = 0; i < hash_len; ++i)
		BOOST_CHECK (hash_dig[i] == etalon2[i]);

	p_crypto_hash_reset (crypto_hash);

	/* Case 3 */

	/* Check string */
	for (int i = 0; i < PCRYPTO_MAX_UPDATES; ++i)
		p_crypto_hash_update (crypto_hash, (const ubyte_t *) "a", 1);

	hash_str = p_crypto_hash_get_string (crypto_hash);

	BOOST_CHECK (strcmp (hash_str, hash3) == 0);
	p_free (hash_str);

	p_crypto_hash_reset (crypto_hash);

	/* Check digest */
	dig_len = hash_len;
	for (int i = 0; i < PCRYPTO_MAX_UPDATES; ++i)
		p_crypto_hash_update (crypto_hash, (const ubyte_t *) "a", 1);

	p_crypto_hash_get_digest (crypto_hash, hash_dig, &dig_len);
	BOOST_CHECK (dig_len == hash_len);

	for (unsigned int i = 0; i < hash_len; ++i)
		BOOST_CHECK (hash_dig[i] == etalon3[i]);

	p_crypto_hash_reset (crypto_hash);

	/* Stress test */
	p_crypto_hash_update (crypto_hash, (const ubyte_t *) long_str, PCRYPTO_STRESS_LENGTH);
	hash_str = p_crypto_hash_get_string (crypto_hash);

	BOOST_CHECK (strcmp (hash_str, hash_stress) == 0);
	p_free (hash_str);

	p_crypto_hash_reset (crypto_hash);

	p_free (long_str);
	p_free (hash_dig);
	p_crypto_hash_free (crypto_hash);
}

BOOST_AUTO_TEST_CASE (pcryptohash_nomem_test)
{
	p_libsys_init ();

	mem_vtable_t vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == true);

	BOOST_CHECK (p_crypto_hash_new (P_HASH_MD5) == NULL);
	BOOST_CHECK (p_crypto_hash_new (P_HASH_SHA1) == NULL);
	BOOST_CHECK (p_crypto_hash_new (P_HASH_GOST) == NULL);

	p_mem_restore_vtable ();

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (pcryptohash_invalid_test)
{
	hash_t	*hash;
	size_t		len;
	ssize_t		md5_len;
	byte_t		*hash_str;
	ubyte_t		*buf;

	p_libsys_init ();

	BOOST_CHECK (p_crypto_hash_new ((hash_kind_t) -1) == NULL);
	BOOST_CHECK (p_crypto_hash_get_length (NULL) == 0);
	BOOST_CHECK (p_crypto_hash_get_string (NULL) == NULL);
	BOOST_CHECK ((int_t) p_crypto_hash_get_type (NULL) == -1);
	p_crypto_hash_free (NULL);

	p_crypto_hash_update (NULL, NULL, 0);
	p_crypto_hash_get_digest (NULL, NULL, NULL);

	p_crypto_hash_get_digest (NULL, NULL, &len);
	BOOST_CHECK (len == 0);

	p_crypto_hash_reset (NULL);

	hash = p_crypto_hash_new (P_HASH_MD5);
	BOOST_CHECK (hash != NULL);

	md5_len = p_crypto_hash_get_length (hash);
	BOOST_CHECK (md5_len > 0);

	buf = (ubyte_t *) p_malloc0 (md5_len);
	BOOST_CHECK (buf != NULL);

	p_crypto_hash_get_digest (hash, buf, &len);
	BOOST_CHECK (len == 0);

	p_crypto_hash_update (hash, (const ubyte_t *) ("abc"), 3);
	len = ((size_t) md5_len) - 1;
	p_crypto_hash_get_digest (hash, buf, &len);
	BOOST_CHECK (len == 0);

	hash_str = p_crypto_hash_get_string (hash);
	BOOST_CHECK (strcmp (hash_str, "900150983cd24fb0d6963f7d28e17f72") == 0);
	p_free (hash_str);

	p_crypto_hash_update (hash, (const ubyte_t *) ("abc"), 3);
	hash_str = p_crypto_hash_get_string (hash);
	BOOST_CHECK (strcmp (hash_str, "900150983cd24fb0d6963f7d28e17f72") == 0);
	p_free (hash_str);

	p_crypto_hash_free (hash);
	p_free (buf);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (md5_test)
{
	const ubyte_t	hash_etalon_1[] = {144,   1,  80, 152,  60, 210,  79, 176,
					   214, 150,  63, 125,  40, 225, 127, 114};
	const ubyte_t	hash_etalon_2[] = {130,  21, 239,   7, 150, 162,  11, 202,
					   170, 225,  22, 211, 135, 108, 102,  74};
	const ubyte_t	hash_etalon_3[] = {119,   7, 214, 174,  78,   2, 124, 112,
					   238, 162, 169,  53, 194,  41, 111,  33};

	p_libsys_init ();

	general_hash_test (P_HASH_MD5,
			   16,
			   "abc",
			   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			   hash_etalon_1,
			   hash_etalon_2,
			   hash_etalon_3,
			   "900150983cd24fb0d6963f7d28e17f72",
			   "8215ef0796a20bcaaae116d3876c664a",
			   "7707d6ae4e027c70eea2a935c2296f21",
			   "e19ea4a77c97fa6c2521ae1ca66982b9");

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (sha1_test)
{
	const ubyte_t	hash_etalon_1[] = {169, 153,  62,  54,  71,   6, 129, 106,
					   186,  62,  37, 113, 120,  80, 194, 108,
					   156, 208, 216, 157};
	const ubyte_t	hash_etalon_2[] = {132, 152,  62,  68,  28,  59, 210, 110,
					   186, 174,  74, 161, 249,  81,  41, 229,
					   229,  70, 112, 241};
	const ubyte_t	hash_etalon_3[] = { 52, 170, 151,  60, 212, 196, 218, 164,
					   246,  30, 235,  43, 219, 173,  39,  49,
					   101,  52,   1, 111};

	p_libsys_init ();

	general_hash_test (P_HASH_SHA1,
			   20,
			   "abc",
			   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			   hash_etalon_1,
			   hash_etalon_2,
			   hash_etalon_3,
			   "a9993e364706816aba3e25717850c26c9cd0d89d",
			   "84983e441c3bd26ebaae4aa1f95129e5e54670f1",
			   "34aa973cd4c4daa4f61eeb2bdbad27316534016f",
			   "56309c2dbe04a348ec801ca5f40b035bad01f907");

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (sha2_224_test)
{
	const ubyte_t	hash_etalon_1[] = { 35,   9, 125,  34,  52,   5, 216,  34, 134,  66,
					   164, 119, 189, 162,  85, 179,  42, 173, 188, 228,
					   189, 160, 179, 247, 227, 108, 157, 167};
	const ubyte_t	hash_etalon_2[] = {117,  56, 139,  22,  81,  39, 118, 204,  93, 186,
					    93, 161, 253, 137,   1,  80, 176, 198,  69,  92,
					   180, 245, 139,  25,  82,  82,  37,  37};
	const ubyte_t	hash_etalon_3[] = { 32, 121,  70,  85, 152,  12, 145, 216, 187, 180,
					   193, 234, 151,  97, 138,  75, 240,  63,  66,  88,
					    25,  72, 178, 238,  78, 231, 173, 103, };

	p_libsys_init ();

	general_hash_test (P_HASH_SHA2_224,
			   28,
			   "abc",
			   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			   hash_etalon_1,
			   hash_etalon_2,
			   hash_etalon_3,
			   "23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7",
			   "75388b16512776cc5dba5da1fd890150b0c6455cb4f58b1952522525",
			   "20794655980c91d8bbb4c1ea97618a4bf03f42581948b2ee4ee7ad67",
			   "4cf3d45b57e0d54981c4d86954e8378168d5a9f6ceab9e0aae5dd2f6");

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (sha2_256_test)
{
	const ubyte_t	hash_etalon_1[] = {186, 120,  22, 191, 143,   1, 207, 234,
					    65,  65,  64, 222,  93, 174,  34,  35,
					   176,   3,  97, 163, 150,  23, 122, 156,
					   180,  16, 255,  97, 242,   0,  21, 173};
	const ubyte_t	hash_etalon_2[] = { 36, 141, 106,  97, 210,   6,  56, 184,
					   229, 192,  38, 147,  12,  62,  96,  57,
					   163,  60, 228,  89, 100, 255,  33, 103,
					   246, 236, 237, 212,  25, 219,   6, 193};
	const ubyte_t	hash_etalon_3[] = {205, 199, 110,  92, 153,  20, 251, 146,
					   129, 161, 199, 226, 132, 215,  62, 103,
					   241, 128, 154,  72, 164, 151,  32,  14,
					     4, 109,  57, 204, 199,  17,  44, 208};

	p_libsys_init ();

	general_hash_test (P_HASH_SHA2_256,
			   32,
			   "abc",
			   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			   hash_etalon_1,
			   hash_etalon_2,
			   hash_etalon_3,
			   "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
			   "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1",
			   "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0",
			   "4c2d7749e1b711ca652fda20dd29fe378fd9988f19eadadfa570682e2c55349f");

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (sha2_384_test)
{
	const ubyte_t	hash_etalon_1[] = {203,   0, 117,  63,  69, 163,  94, 139, 181, 160,  61, 105,
					   154, 198,  80,   7,  39,  44,  50, 171,  14, 222, 209,  99,
					    26, 139,  96,  90,  67, 255,  91, 237, 128, 134,   7,  43,
					   161, 231, 204,  35,  88, 186, 236, 161,  52, 200,  37, 167};
	const ubyte_t	hash_etalon_2[] = { 51, 145, 253, 221, 252, 141, 199,  57,  55,   7, 166,  91,
					    27,  71,   9,  57, 124, 248, 177, 209,  98, 175,   5, 171,
					   254, 143,  69,  13, 229, 243, 107, 198, 176,  69,  90, 133,
					    32, 188,  78, 111,  95, 233,  91,  31, 227, 200,  69,  43};
	const ubyte_t	hash_etalon_3[] = {157,  14,  24,   9, 113, 100, 116, 203,   8, 110, 131,  78,
					    49,  10,  74,  28, 237,  20, 158, 156,   0, 242,  72,  82,
					   121, 114, 206, 197, 112,  76,  42,  91,   7, 184, 179, 220,
					    56, 236, 196, 235, 174, 151, 221, 216, 127,  61, 137, 133};

	p_libsys_init ();

	general_hash_test (P_HASH_SHA2_384,
			   48,
			   "abc",
			   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			   hash_etalon_1,
			   hash_etalon_2,
			   hash_etalon_3,
			   "cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7",
			   "3391fdddfc8dc7393707a65b1b4709397cf8b1d162af05abfe8f450de5f36bc6b0455a8520bc4e6f5fe95b1fe3c8452b",
			   "9d0e1809716474cb086e834e310a4a1ced149e9c00f248527972cec5704c2a5b07b8b3dc38ecc4ebae97ddd87f3d8985",
			   "533e016fd92dd8a8c339328bb5401c3e700e27cd72d8230059e1d4583a506fe8187607bf899a86961af2bf5521b359eb");

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (sha2_512_test)
{
	const ubyte_t	hash_etalon_1[] = {221, 175,  53, 161, 147,  97, 122, 186,
					   204,  65, 115,  73, 174,  32,  65,  49,
					    18, 230, 250,  78, 137, 169, 126, 162,
					    10, 158, 238, 230,  75,  85, 211, 154,
					    33, 146, 153,  42,  39,  79, 193, 168,
					    54, 186,  60,  35, 163, 254, 235, 189,
					    69,  77,  68,  35, 100,  60, 232,  14,
					    42, 154, 201,  79, 165,  76, 164, 159};
	const ubyte_t	hash_etalon_2[] = { 32,  74, 143, 198, 221, 168,  47,  10,
					    12, 237, 123, 235, 142,   8, 164,  22,
					    87, 193, 110, 244, 104, 178,  40, 168,
					    39, 155, 227,  49, 167,   3, 195,  53,
					   150, 253,  21, 193,  59,  27,   7, 249,
					   170,  29,  59, 234,  87, 120, 156, 160,
					    49, 173, 133, 199, 167,  29, 215,   3,
					    84, 236,  99,  18,  56, 202,  52,  69};
	const ubyte_t	hash_etalon_3[] = {231,  24,  72,  61,  12, 231, 105, 100,
					    78,  46,  66, 199, 188,  21, 180,  99,
					   142,  31, 152, 177,  59,  32,  68,  40,
					    86,  50, 168,   3, 175, 169, 115, 235,
					   222,  15, 242,  68, 135, 126, 166,  10,
					    76, 176,  67,  44, 229, 119, 195,  27,
					   235,   0, 156,  92,  44,  73, 170,  46,
					    78, 173, 178,  23, 173, 140, 192, 155};

	p_libsys_init ();

	general_hash_test (P_HASH_SHA2_512,
			   64,
			   "abc",
			   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			   hash_etalon_1,
			   hash_etalon_2,
			   hash_etalon_3,
			   "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f",
			   "204a8fc6dda82f0a0ced7beb8e08a41657c16ef468b228a8279be331a703c33596fd15c13b1b07f9aa1d3bea57789ca031ad85c7a71dd70354ec631238ca3445",
			   "e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973ebde0ff244877ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b",
			   "411525772d02eef0e2ce1107d89b79b8cf6d704e88d4509f726c963d411df6df178c1c9473718f70b0e06c2fda6a9c25f6c91a925849f372634d5f63e6047a20");

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (sha3_224_test)
{
	const ubyte_t	hash_etalon_1[] = {230,  66, 130,  76,  63, 140, 242,  74, 208, 146,
					    52, 238, 125,  60, 118, 111, 201, 163, 165,  22,
					   141,  12, 148, 173, 115, 180, 111, 223};
	const ubyte_t	hash_etalon_2[] = {138,  36,  16, 139,  21,  74, 218,  33, 201, 253,
					    85, 116,  73,  68, 121, 186,  92, 126, 122, 183,
					   110, 242, 100, 234, 208, 252, 206,  51};
	const ubyte_t	hash_etalon_3[] = {214, 147,  53, 185,  51,  37,  25,  46,  81, 106,
					   145,  46, 109,  25, 161,  92, 181,  28, 110, 213,
					   193,  82,  67, 231, 167, 253, 101,  60};

	p_libsys_init ();

	general_hash_test (P_HASH_SHA3_224,
			   28,
			   "abc",
			   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			   hash_etalon_1,
			   hash_etalon_2,
			   hash_etalon_3,
			   "e642824c3f8cf24ad09234ee7d3c766fc9a3a5168d0c94ad73b46fdf",
			   "8a24108b154ada21c9fd5574494479ba5c7e7ab76ef264ead0fcce33",
			   "d69335b93325192e516a912e6d19a15cb51c6ed5c15243e7a7fd653c",
			   "425fbad801bf675651dcf61af1138831480b562e714c70a2a0050ad3");

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (sha3_256_test)
{
	const ubyte_t	hash_etalon_1[] = { 58, 152,  93, 167,  79, 226,  37, 178,
					     4,  92,  23,  45, 107, 211, 144, 189,
					   133,  95,   8, 110,  62, 157,  82,  91,
					    70, 191, 226,  69,  17,  67,  21,  50};
	const ubyte_t	hash_etalon_2[] = { 65, 192, 219, 162, 169, 214,  36,   8,
					    73,  16,   3, 118, 168,  35,  94,  44,
					   130, 225, 185, 153, 138, 153, 158,  33,
					   219,  50, 221, 151,  73, 109,  51, 118};
	const ubyte_t	hash_etalon_3[] = { 92, 136, 117, 174,  71,  74,  54,  52,
					   186,  79, 213,  94, 200,  91, 255, 214,
					    97, 243,  42, 202, 117, 198, 214, 153,
					   208, 205, 203, 108,  17,  88, 145, 193};

	p_libsys_init ();

	general_hash_test (P_HASH_SHA3_256,
			   32,
			   "abc",
			   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			   hash_etalon_1,
			   hash_etalon_2,
			   hash_etalon_3,
			   "3a985da74fe225b2045c172d6bd390bd855f086e3e9d525b46bfe24511431532",
			   "41c0dba2a9d6240849100376a8235e2c82e1b9998a999e21db32dd97496d3376",
			   "5c8875ae474a3634ba4fd55ec85bffd661f32aca75c6d699d0cdcb6c115891c1",
			   "e37ed9f31da3d61740e04c3124a2da5dbe8be0a2ef5c8b5932d45eb1958219e2");

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (sha3_384_test)
{
	const ubyte_t	hash_etalon_1[] = {236,   1,  73, 130, 136,  81, 111, 201,  38,  69, 159,  88,
					   226, 198, 173, 141, 249, 180, 115, 203,  15, 192, 140,  37,
					   150, 218, 124, 240, 228, 155, 228, 178, 152, 216, 140, 234,
					   146, 122, 199, 245,  57, 241, 237, 242,  40,  55, 109,  37};
	const ubyte_t	hash_etalon_2[] = {153,  28, 102,  87,  85, 235,  58,  75, 107, 189, 251, 117,
					   199, 138,  73,  46, 140,  86, 162,  44,  92,  77, 126,  66,
					   155, 253, 188,  50, 185, 212, 173,  90, 160,  74,  31,   7,
					   110,  98, 254, 161, 158, 239,  81, 172, 208, 101, 124,  34};
	const ubyte_t	hash_etalon_3[] = {238, 233, 226,  77, 120, 193, 133,  83,  55, 152,  52,  81,
					   223, 151, 200, 173, 158, 237, 242,  86, 198,  51,  79, 142,
					   148, 141,  37,  45,  94,  14, 118, 132, 122, 160, 119,  77,
					   219, 144, 168,  66,  25,  13,  44,  85, 139,  75, 131,  64};

	p_libsys_init ();

	general_hash_test (P_HASH_SHA3_384,
			   48,
			   "abc",
			   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			   hash_etalon_1,
			   hash_etalon_2,
			   hash_etalon_3,
			   "ec01498288516fc926459f58e2c6ad8df9b473cb0fc08c2596da7cf0e49be4b298d88cea927ac7f539f1edf228376d25",
			   "991c665755eb3a4b6bbdfb75c78a492e8c56a22c5c4d7e429bfdbc32b9d4ad5aa04a1f076e62fea19eef51acd0657c22",
			   "eee9e24d78c1855337983451df97c8ad9eedf256c6334f8e948d252d5e0e76847aa0774ddb90a842190d2c558b4b8340",
			   "3836508de3aa893ad8bd18df238a79e534bc55a6fae84a557bde0820ccfc3ad58e3eaab29a7d0d3bfc071c6d69b2e9d3");

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (sha3_512_test)
{
	const ubyte_t	hash_etalon_1[] = {183,  81, 133,  11,  26,  87,  22, 138,
					    86, 147, 205, 146,  75, 107,   9, 110,
					     8, 246,  33, 130, 116,  68, 247,  13,
					   136,  79,  93,   2,  64, 210, 113,  46,
					    16, 225,  22, 233,  25,  42, 243, 201,
					    26, 126, 197, 118,  71, 227, 147,  64,
					    87,  52,  11,  76, 244,   8, 213, 165,
					   101, 146, 248,  39,  78, 236,  83, 240};
	const ubyte_t	hash_etalon_2[] = {  4, 163, 113, 232,  78, 207, 181, 184,
					   183, 124, 180, 134,  16, 252, 168,  24,
					    45, 212,  87, 206, 111,  50, 106,  15,
					   211, 215, 236,  47,  30, 145,  99, 109,
					   238, 105,  31, 190,  12, 152,  83,   2,
					   186,  27,  13, 141, 199, 140,   8,  99,
					    70, 181,  51, 180, 156,   3,  13, 153,
					   162, 125, 175,  17,  57, 214, 231,  94};
	const ubyte_t	hash_etalon_3[] = { 60,  58, 135, 109, 161,  64,  52, 171,
					    96,  98, 124,   7, 123, 185, 143, 126,
					    18,  10,  42,  83, 112,  33,  45, 255,
					   179,  56,  90,  24, 212, 243, 136,  89,
					   237,  49,  29,  10, 157,  81,  65, 206,
					   156, 197, 198, 110, 230, 137, 178, 102,
					   168, 170,  24, 172, 232,  40,  42,  14,
					    13, 181, 150, 201,  11,  10, 123, 135};

	p_libsys_init ();

	general_hash_test (P_HASH_SHA3_512,
			   64,
			   "abc",
			   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			   hash_etalon_1,
			   hash_etalon_2,
			   hash_etalon_3,
			   "b751850b1a57168a5693cd924b6b096e08f621827444f70d884f5d0240d2712e10e116e9192af3c91a7ec57647e3934057340b4cf408d5a56592f8274eec53f0",
			   "04a371e84ecfb5b8b77cb48610fca8182dd457ce6f326a0fd3d7ec2f1e91636dee691fbe0c985302ba1b0d8dc78c086346b533b49c030d99a27daf1139d6e75e",
			   "3c3a876da14034ab60627c077bb98f7e120a2a5370212dffb3385a18d4f38859ed311d0a9d5141ce9cc5c66ee689b266a8aa18ace8282a0e0db596c90b0a7b87",
			   "16f59fe0b4344af86b37eb145afe41e9dadb45279d074c5bf5c649dd3d2952e47c0ac3a59ea19dc8395d04e8a72fddd9307b839c35fc4bc44a0463003b80dcf1");

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (gost3411_94_test)
{
	hash_t	*gost3411_94_hash;
	byte_t		*hash_str;
	const ubyte_t	hash_etalon_1[] = {177, 196, 102, 211, 117,  25, 184,  46,
					   131,  25, 129, 159, 243,  37, 149, 224,
					    71, 162, 140, 182, 248,  62, 255,  28,
					   105,  22, 168,  21, 166,  55, 255, 250};
	const ubyte_t	hash_etalon_2[] = { 71,  26, 186,  87, 166,  10, 119,  13,
					    58, 118,  19,   6,  53, 193, 251, 234,
					    78, 241,  77, 229,  31, 120, 180, 174,
					    87, 221, 137,  59,  98, 245,  82,   8};
	const ubyte_t	hash_etalon_3[] = { 92,   0, 204, 194, 115,  76, 221,  51,
					    50, 211, 212, 116, 149, 118, 227, 193,
					   167, 219, 175,  14, 126, 167,  78, 159,
					   166,   2,  65,  60, 144, 161,  41, 250};

	p_libsys_init ();

	general_hash_test (P_HASH_GOST,
			   32,
			   "This is message, length=32 bytes",
			   "Suppose the original message has length = 50 bytes",
			   hash_etalon_1,
			   hash_etalon_2,
			   hash_etalon_3,
			   "b1c466d37519b82e8319819ff32595e047a28cb6f83eff1c6916a815a637fffa",
			   "471aba57a60a770d3a76130635c1fbea4ef14de51f78b4ae57dd893b62f55208",
			   "5c00ccc2734cdd3332d3d4749576e3c1a7dbaf0e7ea74e9fa602413c90a129fa",
			   "110ddcb6697d508710c64a62f39e7f202d1ffa20314011a0ebaad1281583d77e");

	gost3411_94_hash = p_crypto_hash_new (P_HASH_GOST);

	BOOST_REQUIRE (gost3411_94_hash != NULL);

	/* Repeat test */
	p_crypto_hash_update (gost3411_94_hash, (const ubyte_t *) "message digest", 14);
	p_crypto_hash_update (gost3411_94_hash, (const ubyte_t *) "message digest", 14);
	p_crypto_hash_update (gost3411_94_hash, (const ubyte_t *) "message digest", 14);
	p_crypto_hash_update (gost3411_94_hash, (const ubyte_t *) "message digest", 14);

	hash_str = p_crypto_hash_get_string (gost3411_94_hash);
	BOOST_CHECK (strcmp (hash_str, "1564064cce4fe1386be063f98d7ab17fc724fa7f02be4fa6847a2162be20d807") == 0);
	p_free (hash_str);

	p_crypto_hash_reset (gost3411_94_hash);
	p_crypto_hash_free (gost3411_94_hash);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
