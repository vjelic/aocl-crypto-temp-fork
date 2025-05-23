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

#include "alcp/cipher/aes_cmac_siv.hh"
#include <gtest/gtest.h>

using namespace alcp::cipher;

TEST(CMACSIV, Initiantiation)
{
    auto alcpCipher = new CipherFactory<iCipherAead>;
    auto siv        = alcpCipher->create("aes-siv-128"); // KeySize is 128 bits
    if (siv == nullptr) {
        delete alcpCipher;
        FAIL();
    }
    delete alcpCipher;
}

TEST(CMACSIV, setKeys)
{
    const int cKeySize          = 16;
    Uint8     key[cKeySize * 2] = {};
    auto      alcpCipher        = new CipherFactory<iCipherAead>;
    auto      siv = alcpCipher->create("aes-siv-128"); // KeySize is 128 bits

    if (siv == nullptr) {
        delete alcpCipher;
        FAIL();
    }
    alc_error_t err = siv->init(key, cKeySize * 8, nullptr, 0);
    EXPECT_TRUE(err == ALC_ERROR_NONE);

    delete alcpCipher;
}

TEST(CMACSIV, addAdditionalInput)
{
    const int cKeySize = 16, cAadSize = 32;
    Uint8     key[cKeySize * 2] = {};
    Uint8     aad[cAadSize]     = {};
    auto      alcpCipher        = new CipherFactory<iCipherAead>;
    auto      siv = alcpCipher->create("aes-siv-128"); // KeySize is 128 bits

    if (siv == nullptr) {
        delete alcpCipher;
        FAIL();
    }
    alc_error_t err = siv->init(key, cKeySize * 8, nullptr, 0);
    EXPECT_TRUE(err == ALC_ERROR_NONE);
    err = siv->setAad(aad, cAadSize);
    EXPECT_TRUE(err == ALC_ERROR_NONE);

    delete alcpCipher;
}

TEST(CMACSIV, encTest1)
{
    const int cKeySize = 16, cAadSize = 24, cPtSize = 14, padLen = 2;
    Uint8     aad[cAadSize] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
                                0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 };
    Uint8     key[cKeySize * 2] = {
        0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8, 0xf7, 0xf6, 0xf5,
        0xf4, 0xf3, 0xf2, 0xf1, 0xf0, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5,
        0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
    };
    Uint8 pt[cPtSize + padLen] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
                                   0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
                                   0xdd, 0xee, 0x00, 0x00 };
    std::vector<Uint8> ct_exp  = { 0x40, 0xc0, 0x2b, 0x96, 0x90, 0xc4, 0xdc,
                                   0x04, 0xda, 0xef, 0x7f, 0x6a, 0xfe, 0x5c };
    std::vector<Uint8> ct_act(cPtSize + padLen, 0);

    std::vector<Uint8> v_exp = {
        0x85, 0x63, 0x2d, 0x07, 0xc6, 0xe8, 0xf3, 0x7f,
        0x95, 0x0a, 0xcd, 0x32, 0x0a, 0x2e, 0xcc, 0x93
    };
    std::vector<Uint8> v_act(16, 0);
    auto               alcpCipher = new CipherFactory<iCipherAead>;
    auto siv = alcpCipher->create("aes-siv-128"); // KeySize is 128 bits

    if (siv == nullptr) {
        delete alcpCipher;
        FAIL();
    }
    alc_error_t err = siv->init(key, cKeySize * 8, nullptr, 0);
    EXPECT_TRUE(err == ALC_ERROR_NONE);

    err = siv->setAad(aad, cAadSize);
    EXPECT_TRUE(err == ALC_ERROR_NONE);

    // err = siv->setPaddingLen(padLen);
    // EXPECT_TRUE(err == ALC_ERROR_NONE);

    siv->encrypt(pt, &ct_act[0], cPtSize);
    std::vector<Uint8> cmp_act(&ct_act.at(0), &ct_act.at(cPtSize));
    EXPECT_EQ(ct_exp, cmp_act);

    err = siv->getTag(&v_act[0], v_act.size());
    EXPECT_TRUE(err == ALC_ERROR_NONE);
    EXPECT_EQ(v_exp, v_act);

    EXPECT_TRUE(err == ALC_ERROR_NONE);

    delete alcpCipher;
}

TEST(CMACSIV, encTest2)
{
    const int cKeySize = 16, cPtSize = 47;
    Uint8     aad1[] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
        0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0xde, 0xad, 0xda, 0xda,
        0xde, 0xad, 0xda, 0xda, 0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa,
        0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00
    };
    Uint8 aad2[] = {
        0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xa0
    };
    Uint8 nonce[]           = { 0x09, 0xf9, 0x11, 0x02, 0x9d, 0x74, 0xe3, 0x5b,
                                0xd8, 0x41, 0x56, 0xc5, 0x63, 0x56, 0x88, 0xc0 };
    Uint8 key[cKeySize * 2] = {
        0x7f, 0x7e, 0x7d, 0x7c, 0x7b, 0x7a, 0x79, 0x78, 0x77, 0x76, 0x75,
        0x74, 0x73, 0x72, 0x71, 0x70, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45,
        0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f
    };
    Uint8 pt[] = { 0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x73, 0x6f,
                   0x6d, 0x65, 0x20, 0x70, 0x6c, 0x61, 0x69, 0x6e, 0x74, 0x65,
                   0x78, 0x74, 0x20, 0x74, 0x6f, 0x20, 0x65, 0x6e, 0x63, 0x72,
                   0x79, 0x70, 0x74, 0x20, 0x75, 0x73, 0x69, 0x6e, 0x67, 0x20,
                   0x53, 0x49, 0x56, 0x2d, 0x41, 0x45, 0x53, 0 };

    std::vector<Uint8> ct_exp = {
        0xcb, 0x90, 0x0f, 0x2f, 0xdd, 0xbe, 0x40, 0x43, 0x26, 0x60, 0x19, 0x65,
        0xc8, 0x89, 0xbf, 0x17, 0xdb, 0xa7, 0x7c, 0xeb, 0x09, 0x4f, 0xa6, 0x63,
        0xb7, 0xa3, 0xf7, 0x48, 0xba, 0x8a, 0xf8, 0x29, 0xea, 0x64, 0xad, 0x54,
        0x4a, 0x27, 0x2e, 0x9c, 0x48, 0x5b, 0x62, 0xa3, 0xfd, 0x5c, 0x0d
    };
    std::vector<Uint8> ct_act(cPtSize + 1, 0);

    std::vector<Uint8> v_exp = {
        0x7b, 0xdb, 0x6e, 0x3b, 0x43, 0x26, 0x67, 0xeb,
        0x06, 0xf4, 0xd1, 0x4b, 0xff, 0x2f, 0xbd, 0x0f
    };
    std::vector<Uint8> v_act(16, 0);
    auto               alcpCipher = new CipherFactory<iCipherAead>;
    auto siv = alcpCipher->create("aes-siv-128"); // KeySize is 128 bits

    if (siv == nullptr) {
        delete alcpCipher;
        FAIL();
    }
    alc_error_t err = siv->init(key, cKeySize * 8, nullptr, 0);
    EXPECT_TRUE(err == ALC_ERROR_NONE);

    err = siv->setAad(aad1, sizeof(aad1));
    EXPECT_TRUE(err == ALC_ERROR_NONE);

    err = siv->setAad(aad2, sizeof(aad2));
    EXPECT_TRUE(err == ALC_ERROR_NONE);

    err = siv->setAad(nonce, sizeof(nonce));
    EXPECT_TRUE(err == ALC_ERROR_NONE);

    // err = siv->setPaddingLen(1);
    // EXPECT_TRUE(err);

    siv->encrypt(pt, &ct_act[0], sizeof(pt) - 1);
    std::vector<Uint8> cmp_act(&ct_act.at(0), &ct_act.at(cPtSize));
    EXPECT_EQ(ct_exp, cmp_act);

    err = siv->getTag(&v_act[0], v_act.size());
    EXPECT_TRUE(err == ALC_ERROR_NONE);
    EXPECT_EQ(v_exp, v_act);

    delete alcpCipher;
}

// FIXME: To bringup decrypt test, proper padding support is needed from API
// level
#if 0
TEST(CMACSIV, decTest1)
{
    const int cKeySize = 16, cAadSize = 24, cPtSize = 14;
    Uint8     aad[cAadSize] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
                                0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 };
    Uint8     key[cKeySize * 2] = {
        0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8, 0xf7, 0xf6, 0xf5,
        0xf4, 0xf3, 0xf2, 0xf1, 0xf0, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5,
        0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
    };
    std::vector<Uint8> pt_exp = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                  0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee };
    std::vector<Uint8> ct     = { 0x40, 0xc0, 0x2b, 0x96, 0x90, 0xc4, 0xdc,
                                  0x04, 0xda, 0xef, 0x7f, 0x6a, 0xfe, 0x5c };
    std::vector<Uint8> pt_act(cPtSize + 2, 0);

    std::vector<Uint8> v_exp = {
        0x85, 0x63, 0x2d, 0x07, 0xc6, 0xe8, 0xf3, 0x7f,
        0x95, 0x0a, 0xcd, 0x32, 0x0a, 0x2e, 0xcc, 0x93
    };
    std::vector<Uint8> v_act(16, 0);
    auto               alcpCipher = new CipherFactory<iCipherAead>;
    auto siv = alcpCipher->create("aes-siv-128"); // KeySize is 128 bits

    if (siv == nullptr) {
        delete alcpCipher;
        FAIL();
    }
    alc_error_t err = siv->init(key, cKeySize * 8, nullptr, 0);
    EXPECT_TRUE(err == ALC_ERROR_NONE);

    err = siv->setAad(aad, cAadSize);
    EXPECT_TRUE(err == ALC_ERROR_NONE);

    // err = siv->setPaddingLen(padLen);
    // EXPECT_TRUE(err == ALC_ERROR_NONE);

    siv->decrypt(&ct[0], &pt_act[0], cPtSize);
    std::vector<Uint8> cmp_act(&pt_act.at(0), &pt_act.at(cPtSize));
    EXPECT_EQ(pt_exp, cmp_act);

    err = siv->getTag(&v_act[0], v_act.size());
    EXPECT_TRUE(err == ALC_ERROR_NONE);
    EXPECT_EQ(v_exp, v_act);

    EXPECT_TRUE(err == ALC_ERROR_NONE);

    delete alcpCipher;
}

TEST(CMACSIV, decTest2)
{
    const int cKeySize = 16, cPtSize = 47;
    Uint8     aad1[] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
        0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0xde, 0xad, 0xda, 0xda,
        0xde, 0xad, 0xda, 0xda, 0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa,
        0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00
    };
    Uint8 aad2[] = {
        0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xa0
    };
    Uint8 nonce[]           = { 0x09, 0xf9, 0x11, 0x02, 0x9d, 0x74, 0xe3, 0x5b,
                                0xd8, 0x41, 0x56, 0xc5, 0x63, 0x56, 0x88, 0xc0 };
    Uint8 key[cKeySize * 2] = {
        0x7f, 0x7e, 0x7d, 0x7c, 0x7b, 0x7a, 0x79, 0x78, 0x77, 0x76, 0x75,
        0x74, 0x73, 0x72, 0x71, 0x70, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45,
        0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f
    };
    std::vector<Uint8> pt_exp = {
        0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x73, 0x6f, 0x6d, 0x65,
        0x20, 0x70, 0x6c, 0x61, 0x69, 0x6e, 0x74, 0x65, 0x78, 0x74, 0x20, 0x74,
        0x6f, 0x20, 0x65, 0x6e, 0x63, 0x72, 0x79, 0x70, 0x74, 0x20, 0x75, 0x73,
        0x69, 0x6e, 0x67, 0x20, 0x53, 0x49, 0x56, 0x2d, 0x41, 0x45, 0x53
    };

    std::vector<Uint8> ct = { 0xcb, 0x90, 0x0f, 0x2f, 0xdd, 0xbe, 0x40, 0x43,
                              0x26, 0x60, 0x19, 0x65, 0xc8, 0x89, 0xbf, 0x17,
                              0xdb, 0xa7, 0x7c, 0xeb, 0x09, 0x4f, 0xa6, 0x63,
                              0xb7, 0xa3, 0xf7, 0x48, 0xba, 0x8a, 0xf8, 0x29,
                              0xea, 0x64, 0xad, 0x54, 0x4a, 0x27, 0x2e, 0x9c,
                              0x48, 0x5b, 0x62, 0xa3, 0xfd, 0x5c, 0x0d, 0 };
    std::vector<Uint8> pt_act(cPtSize + 1, 0);

    std::vector<Uint8> v_exp = {
        0x7b, 0xdb, 0x6e, 0x3b, 0x43, 0x26, 0x67, 0xeb,
        0x06, 0xf4, 0xd1, 0x4b, 0xff, 0x2f, 0xbd, 0x0f
    };
    std::vector<Uint8> v_act(16, 0);
    auto               alcpCipher = new CipherFactory<iCipherAead>;
    auto siv = alcpCipher->create("aes-siv-128"); // KeySize is 128 bits

    if (siv == nullptr) {
        delete alcpCipher;
        FAIL();
    }
    alc_error_t err = siv->init(key, cKeySize * 8, nullptr, 0);
    EXPECT_TRUE(err == ALC_ERROR_NONE);

    err = siv->setAad(aad1, sizeof(aad1));
    EXPECT_TRUE(err == ALC_ERROR_NONE);

    err = siv->setAad(aad2, sizeof(aad2));
    EXPECT_TRUE(err == ALC_ERROR_NONE);

    err = siv->setAad(nonce, sizeof(nonce));
    EXPECT_TRUE(err == ALC_ERROR_NONE);

    // err = siv->setPaddingLen(1);
    // EXPECT_TRUE(err == ALC_ERROR_NONE);

    siv->decrypt(&ct[0], &pt_act[0], cPtSize);
    std::vector<Uint8> cmp_act(&pt_act.at(0), &pt_act.at(cPtSize));
    EXPECT_EQ(pt_exp, cmp_act);

    err = siv->getTag(&v_act[0], v_act.size());
    EXPECT_TRUE(err == ALC_ERROR_NONE);
    EXPECT_EQ(v_exp, v_act);
    delete alcpCipher;
}
#endif
