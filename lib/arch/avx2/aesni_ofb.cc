/*
 * Copyright (C) 2022-2024, Advanced Micro Devices. All rights reserved.
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

#include "alcp/cipher/aes.hh"
#include "alcp/cipher/aesni.hh"
#include "alcp/cipher/aesni_core.hh"
#include "avx2.hh"

#include <immintrin.h>

namespace alcp::cipher::aesni {

/*
 *    OFB mode encrypt and decrypt is same.
 *   We re-use cryptOfb() for both
 */
alc_error_t
__crypt_ofb(const Uint8* pInputText,  // ptr to inputText
            Uint8*       pOutputText, // ptr to outputtext
            Uint64       len,         // message length in bytes
            const Uint8* pKey,        // ptr to Key
            int          nRounds,     // No. of rounds
            Uint8*       pIv          // ptr to Initialization Vector
)
{
    alc_error_t err    = ALC_ERROR_NONE;
    Uint64      blocks = len / Rijndael::cBlockSize;
    Uint64      res    = len % Rijndael::cBlockSize;
    __m128i     a1; // plaintext data

    Uint64 i         = 0;
    auto   p_in_128  = reinterpret_cast<const __m128i*>(pInputText);
    auto   p_out_128 = reinterpret_cast<__m128i*>(pOutputText);
    auto   pkey128   = reinterpret_cast<const __m128i*>(pKey);

    /*
     * load first all keys one-time in xmm registers
     */
    __m128i key_128_0, key_128_1, key_128_2, key_128_3, key_128_4, key_128_5,
        key_128_6, key_128_7, key_128_8, key_128_9, key_128_10, key_128_11,
        key_128_12, key_128_13, key_128_14;

    aesni::alcp_load_key_xmm(pkey128,
                             key_128_0,
                             key_128_1,
                             key_128_2,
                             key_128_3,
                             key_128_4,
                             key_128_5,
                             key_128_6,
                             key_128_7,
                             key_128_8,
                             key_128_9,
                             key_128_10,
                             key_128_11,
                             key_128_12,
                             key_128_13,
                             key_128_14);

    __m128i b1 = _mm_loadu_si128((const __m128i*)pIv);

    if (nRounds == 10) {
        for (i = 0; i < blocks; i++) {
            a1 = _mm_loadu_si128(p_in_128);
            aesni::AesEncryptNoLoad(key_128_0,
                                    key_128_1,
                                    key_128_2,
                                    key_128_3,
                                    key_128_4,
                                    key_128_5,
                                    key_128_6,
                                    key_128_7,
                                    key_128_8,
                                    key_128_9,
                                    key_128_10,
                                    b1);
            a1 = _mm_xor_si128(a1, b1);
            _mm_storeu_si128(p_out_128, a1);
            p_in_128++;
            p_out_128++;
        }
        if (res) {
            a1 = _mm_setzero_si128();

            std::copy((Uint8*)p_in_128, ((Uint8*)p_in_128) + res, (Uint8*)&a1);

            aesni::AesEncryptNoLoad(key_128_0,
                                    key_128_1,
                                    key_128_2,
                                    key_128_3,
                                    key_128_4,
                                    key_128_5,
                                    key_128_6,
                                    key_128_7,
                                    key_128_8,
                                    key_128_9,
                                    key_128_10,
                                    b1);

            std::copy((Uint8*)&a1, ((Uint8*)&a1) + res, (Uint8*)p_out_128);
        }
    } else if (nRounds == 12) {
        for (i = 0; i < blocks; i++) {
            a1 = _mm_loadu_si128(p_in_128);
            aesni::AesEncryptNoLoad(key_128_0,
                                    key_128_1,
                                    key_128_2,
                                    key_128_3,
                                    key_128_4,
                                    key_128_5,
                                    key_128_6,
                                    key_128_7,
                                    key_128_8,
                                    key_128_9,
                                    key_128_10,
                                    key_128_11,
                                    key_128_12,
                                    b1);
            a1 = _mm_xor_si128(a1, b1);
            _mm_storeu_si128(p_out_128, a1);
            p_in_128++;
            p_out_128++;
        }
        if (res) {
            a1 = _mm_setzero_si128();

            std::copy((Uint8*)p_in_128, ((Uint8*)p_in_128) + res, (Uint8*)&a1);

            aesni::AesEncryptNoLoad(key_128_0,
                                    key_128_1,
                                    key_128_2,
                                    key_128_3,
                                    key_128_4,
                                    key_128_5,
                                    key_128_6,
                                    key_128_7,
                                    key_128_8,
                                    key_128_9,
                                    key_128_10,
                                    key_128_11,
                                    key_128_12,
                                    b1);

            std::copy((Uint8*)&a1, ((Uint8*)&a1) + res, (Uint8*)p_out_128);
        }
    } else {
        for (i = 0; i < blocks; i++) {
            a1 = _mm_loadu_si128(p_in_128);
            aesni::AesEncryptNoLoad(key_128_0,
                                    key_128_1,
                                    key_128_2,
                                    key_128_3,
                                    key_128_4,
                                    key_128_5,
                                    key_128_6,
                                    key_128_7,
                                    key_128_8,
                                    key_128_9,
                                    key_128_10,
                                    key_128_11,
                                    key_128_12,
                                    key_128_13,
                                    key_128_14,
                                    b1);
            a1 = _mm_xor_si128(a1, b1);
            _mm_storeu_si128(p_out_128, a1);
            p_in_128++;
            p_out_128++;
        }
        if (res) {
            a1 = _mm_setzero_si128();

            std::copy((Uint8*)p_in_128, ((Uint8*)p_in_128) + res, (Uint8*)&a1);

            aesni::AesEncryptNoLoad(key_128_0,
                                    key_128_1,
                                    key_128_2,
                                    key_128_3,
                                    key_128_4,
                                    key_128_5,
                                    key_128_6,
                                    key_128_7,
                                    key_128_8,
                                    key_128_9,
                                    key_128_10,
                                    key_128_11,
                                    key_128_12,
                                    key_128_13,
                                    key_128_14,
                                    b1);

            std::copy((Uint8*)&a1, ((Uint8*)&a1) + res, (Uint8*)p_out_128);
        }
    }

#ifdef OFB_MULTI_UPDATE
    alcp_storeu_128(reinterpret_cast<__m128i*>(pIv), b1);
#endif

    return err;
}

// multiple keyload code path: currently under experiments.
namespace experimental {
    alc_error_t cryptOfb(const Uint8* pInputText,  // ptr to inputText
                         Uint8*       pOutputText, // ptr to outputtext
                         Uint64       len,         // message length in bytes
                         const Uint8* pKey,        // ptr to Key
                         int          nRounds,     // No. of rounds
                         const Uint8* pIv // ptr to Initialization Vector
    )
    {
        alc_error_t err = ALC_ERROR_NONE;
        __m128i     b1;
        __m128i*    p_in_128  = (__m128i*)pInputText;
        __m128i*    p_out_128 = (__m128i*)pOutputText;
        __m128i*    pkey128   = (__m128i*)pKey;

        b1 = _mm_loadu_si128((__m128i*)pIv);

        /*
         * Effective usage of two 128bit AESENC pipe is not done, since
         * OFB has dependency on previous output.
         */

        Uint64 blocks = len / Rijndael::cBlockSize;
        Uint64 res    = len % Rijndael::cBlockSize;
        for (Uint64 i = 0; i < blocks; i++) {
            __m128i a1 = _mm_loadu_si128(p_in_128); // plaintext

            // 10 rounds
            aesni::AesEncrypt(&b1, pkey128, nRounds);

            a1 = _mm_xor_si128(a1, b1); // cipher = plaintext xor AESENCoutput
            _mm_storeu_si128(p_out_128, a1);
            p_in_128++;
            p_out_128++;
        }

        if (res) {
            __m128i a1 = _mm_setzero_si128();

            std::copy((Uint8*)p_in_128, ((Uint8*)p_in_128) + res, (Uint8*)&a1);

            aesni::AesEncrypt(&b1, pkey128, nRounds);

            a1 = _mm_xor_si128(a1, b1); // cipher = plaintext xor AESENCoutput

            std::copy((Uint8*)&a1, ((Uint8*)&a1) + res, (Uint8*)p_out_128);
        }

        return err;
    }

} // namespace experimental

/*
 * Define the following to use, key-loading per-loop, seems to
 * have lower performance on a single-copy run.
 * TODO: confirm this claim in multi-copy run as well
 */
// #define USE_EXPERIMENTAL 1

alc_error_t
EncryptOfb(const Uint8* pPlainText,  // ptr to plaintext
           Uint8*       pCipherText, // ptr to ciphertext
           Uint64       len,         // message length in bytes
           const Uint8* pKey,        // ptr to Key
           int          nRounds,     // No. of rounds
           Uint8*       pIv          // ptr to Initialization Vector
)
{
    alc_error_t err = ALC_ERROR_NONE;
#if defined(USE_EXPERIMENTAL)
    err = experimental::cryptOfb(pPlainText,  // ptr to inputText
                                 pCipherText, // ptr to outputtext
                                 len,         // message length in bytes
                                 pKey,        // ptr to Key
                                 nRounds,     // No. of rounds
                                 pIv);        // ptr to Initialization Vector
#else
    err = __crypt_ofb(pPlainText,  // ptr to inputText
                      pCipherText, // ptr to outputtext
                      len,         // message length in bytes
                      pKey,        // ptr to Key
                      nRounds,     // No. of rounds
                      pIv);        // ptr to Initialization Vector
#endif
    return err;
}

alc_error_t
DecryptOfb(const Uint8* pCipherText, // ptr to ciphertext
           Uint8*       pPlainText,  // ptr to plaintext
           Uint64       len,         // message length in bytes
           const Uint8* pKey,        // ptr to Key
           int          nRounds,     // No. of rounds
           Uint8*       pIv          // ptr to Initialization Vector
)
{
    alc_error_t err = ALC_ERROR_NONE;

#if defined(USE_EXPERIMENTAL)
    err = experimental::cryptOfb(pCipherText, // ptr to outputtext
                                 pPlainText,  // ptr to inputText
                                 len,         // message length in bytes
                                 pKey,        // ptr to Key
                                 nRounds,     // No. of rounds
                                 pIv);        // ptr to Initialization Vector
#else
    err = __crypt_ofb(pCipherText, // ptr to inputText
                      pPlainText,  // ptr to outputtext
                      len,         // message length in bytes
                      pKey,        // ptr to Key
                      nRounds,     // No. of rounds
                      pIv);        // ptr to Initialization Vector

#endif
    return err;
}

} // namespace alcp::cipher::aesni
