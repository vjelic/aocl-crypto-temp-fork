/*
 * Copyright (C) 2023-2024, Advanced Micro Devices. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once
#ifndef __GTEST_BASE_HH
#define __GTEST_BASE_HH 2

#include "alcp/alcp.h"
#include "csv.hh"
#include "gtest_common.hh"
#include "rng_base.hh"
#include "rsa/alc_rsa.hh"
#include "rsa/rsa.hh"
#include <iostream>
#include <string.h>
#include <vector>
using namespace alcp::testing;
#ifdef USE_IPP
#include "rsa/ipp_rsa.hh"
#endif
#ifdef USE_OSSL
#include "rsa/openssl_rsa.hh"
#endif

#define MAX_LOOP    1600
#define KEY_LEN_MAX 1600
#define INC_LOOP    1
#define START_LOOP  1

/* print params verbosely */
inline void
PrintRsaTestData(alcp_rsa_data_t* data, std::string RsaAlgo)
{
    if (data->m_msg)
        std::cout << "InputData: "
                  << parseBytesToHexStr(data->m_msg, data->m_msg_len)
                  << " Len : " << data->m_msg_len << std::endl;
    if (RsaAlgo.compare("EncryptDecrypt") == 0) {
        if (data->m_encrypted_data)
            std::cout << "EncryptedData: "
                      << parseBytesToHexStr(data->m_encrypted_data,
                                            data->m_msg_len)
                      << " Len : " << data->m_msg_len << std::endl;
        if (data->m_decrypted_data)
            std::cout << "DecryptedData: "
                      << parseBytesToHexStr(data->m_decrypted_data,
                                            data->m_msg_len)
                      << " Len : " << data->m_msg_len << std::endl;
    }
    /* for sign verify tests */
    if (RsaAlgo.compare("SignVerify") == 0
        || RsaAlgo.compare("DigestSignVerify")) {
        if (data->m_signature)
            std::cout << "Signature: "
                      << parseBytesToHexStr(data->m_signature,
                                            ALCP_RSA_SIGNATURE_LEN)
                      << " Len : " << ALCP_RSA_SIGNATURE_LEN << std::endl;
    }
    return;
}

/* to bypass some invalid input cases */
bool
SkipTest(int ret_val, std::string LibStr)
{
    /* for invalid
      inputs, openssl returns RSA_R_DATA_TOO_LARGE_FOR_MODULUS,
      alcp returns ALC_ERROR_NOT_PERMITTED, IPP returns -11 */
    if ((LibStr.compare("ALCP") == 0) && ret_val == ALC_ERROR_INVALID_DATA) {
        if (verbose > 1)
            std::cout << LibStr << ": Invalid case: ret code: " << ret_val
                      << ".. Skipping this test" << std::endl;
        return true;
    }
#if USE_OSSL
    if ((LibStr.compare("OpenSSL") == 0)
        && (ret_val == RSA_R_DATA_TOO_LARGE_FOR_MODULUS
            || ret_val == RSA_R_DATA_TOO_LARGE_FOR_KEY_SIZE)) {
        if (verbose > 1)
            std::cout << LibStr << ": Invalid case: ret code: " << ret_val
                      << ".. Skipping this test" << std::endl;
        return true;
    }
#endif

#if USE_IPP
    if ((LibStr.compare("IPP") == 0) && ret_val == -11) {
        if (verbose > 1)
            std::cout << LibStr << ": Invalid case: ret code: " << ret_val
                      << ".. Skipping this test" << std::endl;
        return true;
    }
#endif

    return false;
}

/* KAT test function for encrypt decrypt / sign verify */
void
Rsa_KAT(std::string             RsaAlgo,
        int                     padding_mode,
        int                     KeySize,
        const alc_digest_info_t dinfo,
        const alc_digest_info_t mgfinfo)
{
    alcp_rsa_data_t data;

    AlcpRsaBase arb;
    std::string LibStr = "ALCP";
    RsaBase*    rb;
    RngBase     rngb;

    rb = &arb;

#ifdef USE_OSSL
    OpenSSLRsaBase orb;
    if (useossl == true) {
        rb     = &orb;
        LibStr = "OpenSSL";
    }
#endif

#ifdef USE_IPP
    IPPRsaBase irb;
    if (useipp == true) {
        rb     = &irb;
        LibStr = "IPP";
    }
#endif

    std::string TestDataFile = "";

    /* default options */
    std::string PaddingModeStr = "";

    rb->m_padding_mode = padding_mode;
    rb->m_rsa_algo     = RsaAlgo;

    if (padding_mode == ALCP_TEST_RSA_NO_PADDING) {
        PaddingModeStr = "no_padding";
    } else if (padding_mode == ALCP_TEST_RSA_PADDING_OAEP) {
        PaddingModeStr = "OAEP";
    } else if (padding_mode == ALCP_TEST_RSA_PADDING_PSS) {
        PaddingModeStr = "PSS";
    } else if (padding_mode == ALCP_TEST_RSA_PADDING_PKCS) {
        PaddingModeStr = "PKCS";
    } else {
        std::cout << "Invalid / Unsupported Rsa Padding mode!" << std::endl;
        FAIL();
    }

    TestDataFile =
        std::string("dataset_RSA_" + RsaAlgo + "_" + std::to_string(KeySize)
                    + "_" + PaddingModeStr + ".csv");

    Csv csv = Csv(TestDataFile);

    if (!csv.m_file_exists) {
        std::cout << "File not found!" << TestDataFile << std::endl;
        FAIL();
    }

    /* Keysize is in bits (1024/2048) */
    KeySize = KeySize / 8;

    while (csv.readNext()) {
        /* input text to be loaded */
        std::vector<Uint8> input_data = csv.getVect("INPUT");
        /*FIXME: reading expected encrypted data is currently only for
         * non-padded modes */
        std::vector<Uint8> encrypted_data_expected =
            csv.getVect("ENCRYPTEDDATA");
        std::vector<Uint8> encrypted_data(KeySize, 0);
        std::vector<Uint8> decrypted_data(KeySize); /* keysize for padded */
        std::vector<Uint8> PubKeyKeyMod(KeySize);
        std::vector<Uint8> signature(KeySize);
        std::vector<Uint8> expected_signature(KeySize);

        /* set randomlen padding for pkcs encrypt/decrypt */
        int                random_pad_len = 0;
        std::vector<Uint8> random_len_padding(KeySize, 0x01);
        if (RsaAlgo.compare("EncryptDecrypt") == 0
            && padding_mode == ALCP_TEST_RSA_PADDING_PKCS) {
            random_pad_len = KeySize - 3 - input_data.size();
            if (random_pad_len > 0) {
                random_len_padding.resize(random_pad_len);
                data.m_random_pad = &(random_len_padding[0]);
            }
        }

        data.m_msg            = getPtr(input_data);
        data.m_pub_key_mod    = getPtr(PubKeyKeyMod);
        data.m_encrypted_data = getPtr(encrypted_data);
        data.m_decrypted_data = getPtr(decrypted_data);
        data.m_msg_len        = input_data.size();
        data.m_key_len        = KeySize;

        rb->m_key_len     = KeySize;
        rb->m_digest_info = dinfo;
        rb->m_mgf_info    = mgfinfo;
        rb->m_hash_len    = dinfo.dt_len / 8;

        std::vector<Uint8> digest(rb->m_hash_len, 0);
        data.m_digest = getPtr(digest);

        /* seed and label */
        std::vector<Uint8> seed(rb->m_hash_len);
        data.m_pseed = getPtr(seed);
        std::vector<Uint8> label(5);
        data.m_label      = getPtr(label);
        data.m_label_size = label.size();

        int ret_val = 0;

        data.m_salt_len = rb->m_key_len - rb->m_hash_len - 2;
        std::vector<Uint8> salt(data.m_salt_len, 0);

        /* for signature generation and verification*/
        if (RsaAlgo.compare("SignVerify") == 0
            || RsaAlgo.compare("DigestSignVerify") == 0) {
            expected_signature = csv.getVect("SIGNATURE");
            data.m_signature   = getPtr(signature);
            /*FIXME: experimenting with Zero length salt for RSA PSS*/
            data.m_salt     = nullptr;
            data.m_salt_len = 0;
            if (padding_mode == ALCP_TEST_RSA_PADDING_PSS
                && RsaAlgo.compare("DigestSignVerify") == 0) {
                data.m_salt = getPtr(salt);
            }
        }

        /* Init and setting keys are common for all modes */
        if (!rb->init()) {
            std::cout << "Error in RSA init" << std::endl;
            FAIL();
        }

        if (LibStr.compare("ALCP") == 0) {
            if (RsaAlgo.compare("SignVerify") == 0) {
                if (!rb->SetPublicKeyBigNum(data)) {
                    std::cout << "Err:RSA set pubkey bignum" << std::endl;
                    FAIL();
                }
                if (!rb->SetPrivateKeyBigNum(data)) {
                    std::cout << "Err:RSA set pvt key bignum" << std::endl;
                    FAIL();
                }
            } else {
                if (!rb->SetPublicKey(data)) {
                    std::cout << "Err:RSA set pubkey" << std::endl;
                    FAIL();
                }
                if (!rb->SetPrivateKey(data)) {
                    std::cout << "Err:RSA set pvt key" << std::endl;
                    FAIL();
                }
            }
        } else {
            if (!rb->SetPublicKeyBigNum(data)) {
                std::cout << "Err:RSA set pubkey bignum" << std::endl;
                FAIL();
            }
            if (!rb->SetPrivateKeyBigNum(data)) {
                std::cout << "Err:RSA set pvt key bignum" << std::endl;
                FAIL();
            }
        }

        /* for signature and verification */
        if (RsaAlgo.compare("DigestSignVerify") == 0) {
            /* this is to first calculate digest and then signing the digest
             * output */
            /* here, add cases for PKCS/PSS */
            if (!rb->Sign(data)) {
                std::cout << "Error in RSA Digest + sign" << std::endl;
                FAIL();
            }
            if (!rb->Verify(data)) {
                std::cout << "Error in RSA Digest + verify" << std::endl;
                FAIL();
            }
            if (padding_mode == ALCP_TEST_RSA_PADDING_PKCS) {
                EXPECT_TRUE(
                    ArraysMatch(expected_signature, signature, KeySize));
            }
        } else if (RsaAlgo.compare("SignVerify") == 0) {
            if (!rb->DigestSign(data)) {
                std::cout << "Error in RSA sign" << std::endl;
                FAIL();
            }
            if (!rb->DigestVerify(data)) {
                std::cout << "Error in RSA verify" << std::endl;
                FAIL();
            }
            EXPECT_TRUE(ArraysMatch(expected_signature, signature, KeySize));
        }

        /* for encrypt / decrypt cases */
        else if (RsaAlgo.compare("EncryptDecrypt") == 0) {
            ret_val = rb->EncryptPubKey(data);
            if (ret_val != 0) {
                std::cout << "Error in RSA EncryptPubKey" << std::endl;
                FAIL();
            }

            ret_val = rb->DecryptPvtKey(data);
            if (ret_val != 0) {
                std::cout << "Error in RSA DecryptPvtKey" << std::endl;
                FAIL();
            }

            /* check if dec val is same as input */
            if (padding_mode == ALCP_TEST_RSA_PADDING_OAEP
                || padding_mode == ALCP_TEST_RSA_PADDING_PKCS) {
                input_data.resize(KeySize);
                EXPECT_TRUE(
                    ArraysMatch(decrypted_data, input_data, input_data.size()));
                data.m_msg_len = KeySize;
                data.m_msg     = getPtr(input_data);
            } else {
                EXPECT_TRUE(ArraysMatch(
                    decrypted_data, input_data, csv, std::string("RSA")));
                /*FIXME: reading expected encrypted data is currently only for
                 * non-padded modes */
                EXPECT_TRUE(ArraysMatch(encrypted_data,
                                        encrypted_data_expected,
                                        csv,
                                        std::string("RSA")));
            }
        }
        if (verbose > 1) {
            PrintRsaTestData(&data, RsaAlgo);
        }
    }
    return;
}

/* RSA Cross tests */
void
Rsa_Cross(std::string             RsaAlgo,
          int                     padding_mode,
          int                     KeySize,
          const alc_digest_info_t dinfo,
          const alc_digest_info_t mgfinfo)
{
    alcp_rsa_data_t   data_main{}, data_ext{};
    int               ret_val_main = 0, ret_val_ext = 0;
    AlcpRsaBase       arb;
    alc_drbg_handle_t handle{};
    alc_drbg_info_t   drbg_info{};
    alc_error_t       err = ALC_ERROR_NONE;

    // FIXME: Better use unique pointer here
    RsaBase *rb_main = {}, *rb_ext = {};

    rb_main                = &arb;
    std::string LibStrMain = "ALCP", LibStrExt = "";

    /* Keysize is in bits */
    KeySize = KeySize / 8;
    std::vector<Uint8> random_len_padding(KeySize, 0x01);
    int                InputSize_Max;

#ifdef USE_OSSL
    OpenSSLRsaBase orb;
    if (useipp == false && useossl == false) {
        printErrors("Defaulting to OpenSSL");
        useossl = true;
    }
    if (useossl) {
        rb_ext    = &orb;
        LibStrExt = "OpenSSL";
    }
#else
    if ((useipp == false && useossl == false) || useossl == true) {
        printErrors("No Lib Selected. OpenSSL also not available");
        FAIL() << "OpenSSL not available, cannot proceed with defaults!";
    }
#endif
#ifdef USE_IPP
    IPPRsaBase irb;
    if (useipp == true) {
        rb_ext    = &irb;
        LibStrExt = "IPP";
    }
#else
    if (useipp == true) {
        printErrors("IPP selected, but not available.");
        FAIL() << "IPP Missing at compile time!";
    }
#endif

    if (rb_ext == nullptr) {
        printErrors("No external lib selected!");
        exit(-1);
    }

    rb_main->m_digest_info = rb_ext->m_digest_info = dinfo;
    rb_main->m_mgf_info = rb_ext->m_mgf_info = mgfinfo;
    rb_main->m_hash_len = rb_ext->m_hash_len = dinfo.dt_len / 8;

    rb_main->m_rsa_algo = rb_ext->m_rsa_algo = RsaAlgo;
    rb_main->m_padding_mode = rb_ext->m_padding_mode = padding_mode;
    if (padding_mode != ALCP_TEST_RSA_NO_PADDING) {
        /* input size should be 0 to m_key_size - 2 * m_hash_len - 2*/
        if (KeySize == 128) {
            InputSize_Max = 62;
        } else
            InputSize_Max = 47;
    } else {
        /* for no padding, input size = key size */
        InputSize_Max = KeySize;
    }

    rb_main->m_key_len = KeySize;
    rb_ext->m_key_len  = KeySize;

    int loop_start = 1;
    if (rb_ext == nullptr) {
        std::cout << "No external lib selected!" << std::endl;
        exit(-1);
    }

    /* use ctr-drbg to randomize the input buffer */
    /* TO DO: maybe parameterize the DRBG type, and params in future? */
    drbg_info.di_algoinfo.ctr_drbg.di_keysize              = 128;
    drbg_info.di_algoinfo.ctr_drbg.use_derivation_function = true;
    drbg_info.di_type                                      = ALC_DRBG_CTR;
    drbg_info.max_entropy_len = drbg_info.max_nonce_len = 16;
    drbg_info.di_rng_sourceinfo.custom_rng              = false;
    drbg_info.di_rng_sourceinfo.di_sourceinfo.rng_info.ri_distrib =
        ALC_RNG_DISTRIB_UNIFORM;
    drbg_info.di_rng_sourceinfo.di_sourceinfo.rng_info.ri_source =
        ALC_RNG_SOURCE_ARCH;
    drbg_info.di_rng_sourceinfo.di_sourceinfo.rng_info.ri_type =
        ALC_RNG_TYPE_DISCRETE;

    err = alcp_drbg_supported(&drbg_info);
    if (alcp_is_error(err)) {
        std::cout << "Hardware Rng support failed. Falling Back to System Rng"
                  << std::endl;

        // Fall back to OS RNG if hardware rng rdrand instruction is not
        // supported.
        drbg_info.di_rng_sourceinfo.di_sourceinfo.rng_info.ri_source =
            ALC_RNG_SOURCE_OS;
        err = alcp_drbg_supported(&drbg_info);
        if (alcp_is_error(err)) {
            std::cout << "Error: alcp_drbg_supported: " << err << std::endl;
            FAIL();
        }
    }

    handle.ch_context = malloc(alcp_drbg_context_size(&drbg_info));
    if (handle.ch_context == nullptr) {
        std::cout << "Error: alcp_drbg_supported: " << std::endl;
        FAIL();
    }
    err = alcp_drbg_request(&handle, &drbg_info);
    if (alcp_is_error(err)) {
        std::cout << "Error: alcp_drbg_request: " << err << std::endl;
        FAIL();
    }
    const int cSecurityStrength = 100;
    err = alcp_drbg_initialize(&handle, cSecurityStrength, NULL, 0);
    if (alcp_is_error(err)) {
        std::cout << "Error: alcp_drbg_initialize: " << err << std::endl;
        FAIL();
    }
#endif

    int InputSize = 0;
    for (int i = loop_start; i < InputSize_Max; i++) {
        /* For non-padded mode, input len will always be KeySize */
        /* over-allocating this to test the misaligned pointers */
        if (padding_mode == ALCP_TEST_RSA_NO_PADDING) {
            InputSize = InputSize_Max + 1;
        } else {
            InputSize = i + 1;
        }
        std::vector<Uint8> input_data(InputSize);

        /* shuffle input vector after each iterations */
        err = alcp_drbg_randomize(&handle,
                                  &(input_data[0]),
                                  input_data.size(),
                                  cSecurityStrength,
                                  NULL,
                                  0);
        if (alcp_is_error(err)) {
            std::cout << "Error: alcp_drbg_randomize on input data: " << err
                      << std::endl;
            FAIL();
        }

        /* set test data for each lib */
        std::vector<Uint8> encrypted_data_main(KeySize);
        std::vector<Uint8> decrypted_data_main(KeySize);
        std::vector<Uint8> PubKeyKeyMod_main(KeySize);
        std::vector<Uint8> encrypted_data_ext(KeySize);
        std::vector<Uint8> decrypted_data_ext(KeySize);
        std::vector<Uint8> PubKeyKeyMod_ext(KeySize);

        std::vector<Uint8> signature_data_main(KeySize);
        std::vector<Uint8> signature_data_ext(KeySize);

        /* set randomlen padding for pkcs encrypt/decrypt */
        int random_pad_len = 0;
        if (RsaAlgo.compare("EncryptDecrypt") == 0
            && padding_mode == ALCP_TEST_RSA_PADDING_PKCS) {
            random_pad_len = KeySize - 3 - input_data.size();
            if (random_pad_len > 0) {
                random_len_padding.resize(random_pad_len);
                data_main.m_random_pad = data_ext.m_random_pad =
                    &(random_len_padding[0]);
            }
        }

        /* misalign if buffers are aligned */
        bool force_misaligned = false;
        if (is_aligned(&(input_data[0]))) {
            data_main.m_msg  = &(input_data[1]);
            data_ext.m_msg   = &(input_data[1]);
            force_misaligned = true;
        } else {
            data_main.m_msg = &(input_data[0]);
            data_ext.m_msg  = &(input_data[0]);
        }

        data_main.m_signature = &(signature_data_main[0]);
        data_ext.m_signature  = &(signature_data_ext[0]);

        data_main.m_pub_key_mod    = &(PubKeyKeyMod_main[0]);
        data_main.m_encrypted_data = &(encrypted_data_main[0]);
        data_main.m_decrypted_data = &(decrypted_data_main[0]);

        data_ext.m_pub_key_mod    = &(PubKeyKeyMod_ext[0]);
        data_ext.m_encrypted_data = &(encrypted_data_ext[0]);
        data_ext.m_decrypted_data = &(decrypted_data_ext[0]);

        data_main.m_key_len = data_ext.m_key_len = KeySize;
        data_main.m_msg_len = data_ext.m_msg_len = input_data.size() - 1;

        std::vector<Uint8> digest(rb_main->m_hash_len, 0);
        data_main.m_digest = data_ext.m_digest = getPtr(digest);

        /* set seed and label for padding mode */
        std::vector<Uint8> seed(rb_main->m_hash_len);
        if (padding_mode == 1) {
            /* shuffle seed data after each iterations */
            err = alcp_drbg_randomize(
                &handle, &(seed[0]), seed.size(), cSecurityStrength, NULL, 0);
            if (alcp_is_error(err)) {
                std::cout << "Error: alcp_drbg_randomize seed data: " << err
                          << std::endl;
                FAIL();
            }
        }

        data_main.m_pseed = data_ext.m_pseed = &(seed[0]);

        /* label length should vary */
        std::vector<Uint8> label(i * KeySize);
        err = alcp_drbg_randomize(
            &handle, &(label[0]), label.size(), cSecurityStrength, NULL, 0);
        if (alcp_is_error(err)) {
            std::cout << "Error: alcp_drbg_randomize label data: " << err
                      << std::endl;
            FAIL();
        }
        data_main.m_label = data_ext.m_label = &(label[0]);
        data_main.m_label_size = data_ext.m_label_size = label.size();

        /* for PSS Digest sign Salt */
        data_main.m_salt_len = data_ext.m_salt_len =
            rb_main->m_key_len - rb_main->m_hash_len - 2;
        std::vector<Uint8> salt(data_main.m_salt_len, 0);

        data_main.m_salt = data_ext.m_salt = nullptr;
        data_main.m_salt_len = data_ext.m_salt_len = 0;

        if (padding_mode == ALCP_TEST_RSA_PADDING_PSS
            && RsaAlgo.compare("DigestSignVerify") == 0) {
            data_main.m_salt = data_ext.m_salt = getPtr(salt);
        }

        /* initialize */
        if (!rb_main->init()) {
            std::cout << "Error in RSA init for " << LibStrMain << std::endl;
            FAIL();
        }
        if (!rb_ext->init()) {
            std::cout << "Error in RSA init for " << LibStrExt << std::endl;
            FAIL();
        }
        /* set public, private keys for both libs */
        if (!rb_main->SetPublicKeyBigNum(data_main)) {
            std::cout << "Error in RSA set pubkey for " << LibStrMain
                      << std::endl;
            FAIL();
        }
        if (!rb_ext->SetPublicKeyBigNum(data_ext)) {
            std::cout << "Error in RSA set pubkey for " << LibStrExt
                      << std::endl;
            FAIL();
        }
        if (!rb_main->SetPrivateKeyBigNum(data_main)) {
            std::cout << "Error in RSA set pvt key for " << LibStrMain
                      << std::endl;
            FAIL();
        }
        if (!rb_ext->SetPrivateKeyBigNum(data_ext)) {
            std::cout << "Error in RSA set pvt key for " << LibStrExt
                      << std::endl;
            FAIL();
        }

        if (RsaAlgo.compare("DigestSignVerify") == 0) {
            /* sign and verify */
            if (!rb_main->Sign(data_main)) {
                std::cout << "Error in RSA Digest + sign for " << LibStrMain
                          << std::endl;
                FAIL();
            }
            if (!rb_ext->Sign(data_ext)) {
                std::cout << "Error in RSA Digest + sign for " << LibStrExt
                          << std::endl;
                FAIL();
            }
            if (!rb_main->Verify(data_main)) {
                std::cout << "Error in RSA Digest + Sign verify for "
                          << LibStrMain << std::endl;
                FAIL();
            }
            if (!rb_ext->Verify(data_ext)) {
                std::cout << "Error in RSA Digest + Sign verify for "
                          << LibStrExt << std::endl;
                FAIL();
            }
            /* check if signature generated by both libraries are same */
            /* this check is not valid for PSS mode where signature will always
             * be non-deterministic */
            if (padding_mode == ALCP_TEST_RSA_PADDING_PKCS) {
                EXPECT_TRUE(ArraysMatch(
                    signature_data_main, signature_data_ext, KeySize));
            }
        } else if (RsaAlgo.compare("SignVerify") == 0) {
            /* sign and verify */
            if (!rb_main->DigestSign(data_main)) {
                std::cout << "Error in RSA sign for " << LibStrMain
                          << std::endl;
                FAIL();
            }
            if (!rb_ext->DigestSign(data_ext)) {
                std::cout << "Error in RSA sign for " << LibStrExt << std::endl;
                FAIL();
            }
            if (!rb_main->DigestVerify(data_main)) {
                std::cout << "Error in RSA verify for " << LibStrMain
                          << std::endl;
                FAIL();
            }
            if (!rb_ext->DigestVerify(data_ext)) {
                std::cout << "Error in RSA verify for " << LibStrExt
                          << std::endl;
                FAIL();
            }
            /* check if signature generated by both libraries are same */
            EXPECT_TRUE(
                ArraysMatch(signature_data_main, signature_data_ext, KeySize));
        } else if (RsaAlgo.compare("EncryptDecrypt") == 0) {
            /* Call encrypt for both libs */
            ret_val_main = rb_main->EncryptPubKey(data_main);
            ret_val_ext  = rb_ext->EncryptPubKey(data_ext);
            if (SkipTest(ret_val_main, LibStrMain)
                && SkipTest(ret_val_ext, LibStrExt))
                continue;
            if (ret_val_main != 0) {
                std::cout << "Error in RSA EncryptPubKey for " << LibStrMain
                          << std::endl;
                FAIL();
            }
            if (ret_val_ext != 0) {
                std::cout << "Error in RSA EncryptPubKey for " << LibStrExt
                          << std::endl;
                FAIL();
            }

            /* Call decrypt for both libs */
            ret_val_main = rb_main->DecryptPvtKey(data_main);
            ret_val_ext  = rb_ext->DecryptPvtKey(data_ext);
            if (SkipTest(ret_val_main, LibStrMain)
                && SkipTest(ret_val_ext, LibStrExt))
                continue;
            if (ret_val_main != 0) {
                std::cout << "Error in RSA EncryptPubKey for " << LibStrMain
                          << std::endl;
                FAIL();
            }
            if (ret_val_ext != 0) {
                std::cout << "Error in RSA EncryptPubKey for " << LibStrExt
                          << std::endl;
                FAIL();
            }
            /* if we are misaligning the input buffer, resize */
            if (force_misaligned) {
                input_data = std::vector<Uint8>(input_data.begin() + 1,
                                                input_data.end());
            }
            /* Now check outputs from both libs */
            if (padding_mode == ALCP_TEST_RSA_PADDING_OAEP
                || padding_mode == ALCP_TEST_RSA_PADDING_PKCS) {
                input_data.resize(KeySize);
                /* compare decrypted output for ext lib vs original input */
                EXPECT_TRUE(ArraysMatch(decrypted_data_main, input_data, i));
                EXPECT_TRUE(ArraysMatch(decrypted_data_ext, input_data, i));
                EXPECT_TRUE(
                    ArraysMatch(decrypted_data_ext, decrypted_data_main, i));
                /* now revert input data to original len after
                 * verification*/
                input_data.resize(i);
            } else {
                /* For non-padded mode, input len will always be KeySize */
                EXPECT_TRUE(ArraysMatch(
                    decrypted_data_main, input_data, InputSize_Max));
                EXPECT_TRUE(
                    ArraysMatch(decrypted_data_ext, input_data, InputSize_Max));
                EXPECT_TRUE(ArraysMatch(
                    decrypted_data_ext, decrypted_data_main, InputSize_Max));
            }
        }

        if (verbose > 1) {
            PrintRsaTestData(&data_main, RsaAlgo);
            PrintRsaTestData(&data_ext, RsaAlgo);
        }
    }

    alcp_drbg_finish(&handle);
    if (handle.ch_context) {
        free(handle.ch_context);
        handle.ch_context = nullptr;
    }

    return;
}