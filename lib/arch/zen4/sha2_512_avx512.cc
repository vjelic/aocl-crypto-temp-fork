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

// FIXME: This code is duplicated from zen3. Similar duplication exists in zen2
// as well. Find a unified architecture
#include "alcp/digest.hh"
#include "alcp/digest/sha512.hh"
#include "alcp/digest/sha_avx256.hh"
#include "config.h"

#include <iostream>
#include <string.h>
#include <x86intrin.h>

#define SHA512_WORDS_IN_128_BIT_VEC                                            \
    2 // Number of sha512 words that can be accomodated in 128 bit vector
#define SHA512_WORDS_IN_256_BIT_VEC                                            \
    4 // Number of sha512 words that can be accomodated in 256 bit vector
#define SHA512_CHUNK_NUM_VECT_AVX                                              \
    8 // Number of avx registers needed to accomodate a sha512 block
#define SHA512_CHUNK_NUM_VECT_AVX2                                             \
    4 // Number of YMM registers needed to accomodate a sha512 block
#define SHA512_STATE_SIZE_IN_256_BIT_VEC                                       \
    2 // Number of avx2 registers needed to STORE THE SHA512 state

#if defined(COMPILER_IS_GCC)
#define UNROLL_8  _Pragma("GCC unroll 8")
#define UNROLL_16 _Pragma("GCC unroll 16")
#define UNROLL_80 _Pragma("GCC unroll 80")
#else
#define UNROLL_8
#define UNROLL_16
#define UNROLL_80
#endif

namespace alcp::digest { namespace zen4 {

    __attribute__((aligned(64))) static const Uint64 sha512_hash_constsx2[] = {
        0x428a2f98d728ae22, 0x7137449123ef65cd, 0x428a2f98d728ae22,
        0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
        0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc, 0x3956c25bf348b538,
        0x59f111f1b605d019, 0x3956c25bf348b538, 0x59f111f1b605d019,
        0x923f82a4af194f9b, 0xab1c5ed5da6d8118, 0x923f82a4af194f9b,
        0xab1c5ed5da6d8118, 0xd807aa98a3030242, 0x12835b0145706fbe,
        0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c,
        0x550c7dc3d5ffb4e2, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
        0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x72be5d74f27b896f,
        0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
        0x9bdc06a725c71235, 0xc19bf174cf692694, 0xe49b69c19ef14ad2,
        0xefbe4786384f25e3, 0xe49b69c19ef14ad2, 0xefbe4786384f25e3,
        0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65, 0x0fc19dc68b8cd5b5,
        0x240ca1cc77ac9c65, 0x2de92c6f592b0275, 0x4a7484aa6ea6e483,
        0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4,
        0x76f988da831153b5, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
        0x983e5152ee66dfab, 0xa831c66d2db43210, 0x983e5152ee66dfab,
        0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
        0xb00327c898fb213f, 0xbf597fc7beef0ee4, 0xc6e00bf33da88fc2,
        0xd5a79147930aa725, 0xc6e00bf33da88fc2, 0xd5a79147930aa725,
        0x06ca6351e003826f, 0x142929670a0e6e70, 0x06ca6351e003826f,
        0x142929670a0e6e70, 0x27b70a8546d22ffc, 0x2e1b21385c26c926,
        0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed,
        0x53380d139d95b3df, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
        0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x650a73548baf63de,
        0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
        0x81c2c92e47edaee6, 0x92722c851482353b, 0xa2bfe8a14cf10364,
        0xa81a664bbc423001, 0xa2bfe8a14cf10364, 0xa81a664bbc423001,
        0xc24b8b70d0f89791, 0xc76c51a30654be30, 0xc24b8b70d0f89791,
        0xc76c51a30654be30, 0xd192e819d6ef5218, 0xd69906245565a910,
        0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a,
        0x106aa07032bbd1b8, 0xf40e35855771202a, 0x106aa07032bbd1b8,
        0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x19a4c116b8d2d0c8,
        0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
        0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8, 0x391c0cb3c5c95a63,
        0x4ed8aa4ae3418acb, 0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb,
        0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3, 0x5b9cca4f7763e373,
        0x682e6ff3d6b2b8a3, 0x748f82ee5defb2fc, 0x78a5636f43172f60,
        0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72,
        0x8cc702081a6439ec, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
        0x90befffa23631e28, 0xa4506cebde82bde9, 0x90befffa23631e28,
        0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
        0xbef9a3f7b2c67915, 0xc67178f2e372532b, 0xca273eceea26619c,
        0xd186b8c721c0c207, 0xca273eceea26619c, 0xd186b8c721c0c207,
        0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178, 0xeada7dd6cde0eb1e,
        0xf57d4f7fee6ed178, 0x06f067aa72176fba, 0x0a637dc5a2c898a6,
        0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae,
        0x1b710b35131c471b, 0x113f9804bef90dae, 0x1b710b35131c471b,
        0x28db77f523047d84, 0x32caab7b40c72493, 0x28db77f523047d84,
        0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
        0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c, 0x4cc5d4becb3e42b6,
        0x597f299cfc657e2a, 0x4cc5d4becb3e42b6, 0x597f299cfc657e2a,
        0x5fcb6fab3ad6faec, 0x6c44198c4a475817, 0x5fcb6fab3ad6faec,
        0x6c44198c4a475817
    };

    static inline void alcp_load_hash_ymm(const __m256i* phash,
                                          __m256i&       hash_256_0,
                                          __m256i&       hash_256_1)
    {
        hash_256_0 = _mm256_load_si256(phash);
        hash_256_1 = _mm256_load_si256(phash + 1);
    }

    static inline void load_data(__m256i      x[SHA512_CHUNK_NUM_VECT_AVX2 * 2],
                                 Uint64       msg_sch_array_1[96],
                                 const Uint8* data)
    {
        const __m256i shuf_mask = _mm256_setr_epi64x(0x0001020304050607ULL,
                                                     0x08090a0b0c0d0e0fULL,
                                                     0x0001020304050607ULL,
                                                     0x08090a0b0c0d0e0fULL);
        __m256i       tmp;
        UNROLL_8

        for (Uint32 i = 0; i < SHA512_CHUNK_NUM_VECT_AVX2 * 2; i++) {
            const Uint32 pos0 = (sizeof(__m256i) / 2) * i;
            const Uint32 pos1 = pos0 + Sha512::cChunkSize;
            x[i]              = _mm256_insertf128_si256(
                x[i], _mm_lddqu_si128((__m128i*)&data[pos1]), 1);
            x[i] = _mm256_insertf128_si256(
                x[i], _mm_lddqu_si128((__m128i*)&data[pos0]), 0);
            x[i] = _mm256_shuffle_epi8(x[i], shuf_mask);

            tmp = _mm256_add_epi64(
                x[i],
                _mm256_loadu_si256(
                    (const __m256i*)(&sha512_hash_constsx2[i * 4])));

            _mm_store_si128((__m128i*)&(msg_sch_array_1[16 + i * 2]),
                            _mm256_extracti128_si256(tmp, 1));
            _mm_store_si128((__m128i*)&(msg_sch_array_1[i * 2]),
                            _mm256_extracti128_si256(tmp, 0));
        }
    }

    static inline void rotate_x(__m256i x[8])
    {
        const __m256i tmp = x[0];

        x[0] = x[1];
        x[1] = x[2];
        x[2] = x[3];
        x[3] = x[4];
        x[4] = x[5];
        x[5] = x[6];
        x[6] = x[7];

        x[7] = tmp;
    }

    static inline void rotate_x(__m128i x[8])
    {
        const __m128i tmp = x[0];
        x[0]              = x[1];
        x[1]              = x[2];
        x[2]              = x[3];
        x[3]              = x[4];
        x[4]              = x[5];
        x[5]              = x[6];
        x[6]              = x[7];

        x[7] = tmp;
    }

    static inline __m256i ROTR(const __m256i reg, int bits)
    {
        __m256i temp1 = _mm256_srli_epi64(reg, bits);
        __m256i temp2 = _mm256_slli_epi64(reg, 64 - bits);
        return temp1 |= temp2;
    }

    inline __m256i sigma0(const __m256i temp)
    {
        return ROTR(temp, 1) ^ ROTR(temp, 8) ^ _mm256_srli_epi64(temp, 7);
    }
    inline __m256i sigma_0_512(const __m256i x0,
                               const __m256i x1,
                               const __m256i x4,
                               const __m256i x5)
    {
        /* x0=[w0,w1], x1 = [w2,w3], x2 = [w4,w5], x4 = [w8,w9],x5 =[w10,w11].
         * These are intial values and reusing them and rotate_x will change
         * their values. But the below code could be understood easily with
         * comments in form of initial values*/

        // Calculation of  sigma_0_512(x) = ROTR(x,1)^ROTR(x,8)^SHR(x,7)

        // temp = [ w1 || w2 ](b0)__[ w1 || w2 ](b1)
        __m256i temp = _mm256_alignr_epi8(x1, x0, 8);

        /* s0 = [ sigma_0_512(w1) || sigma_0_512(w2) ](b0)__
                [ sigma_0_512(w1) || sigma_0_512(w2) ](b1) */
        __m256i s0 = sigma0(temp);

        // From here addition to w9 and w10 also is included in this fcuntion as
        // moving it outside the function cause performance drop with gcc.
        // temp = [ w9 || w10 ](b0)__[ w9 || w10 ](b1)
        temp = _mm256_alignr_epi8(x5, x4, 8);

        // temp =  [ w0+w9|| w1+w10 ]
        temp = _mm256_add_epi64(x0, temp);

        /* temp = [ sigma_0_512(w1)+w0+w9 || sigma_0_512(w2)+w1+10 ](b0)__
           [ sigma_0_512(w1)+w0+w9 || sigma_0_512(w2)+w1+10 ](b1)  */
        temp = _mm256_add_epi64(s0, temp);

        return temp;
    }

    inline __m256i sigma_1_512(const __m256i x7)
    {
        /*  x7 = [w14,w15]
         * These are intial values and reusing them and rotate_x will change
         * their values. But the below code could be understood easily with
         * comments in form of initial values*/
        /*
           [(ROTR(w14,19) ^ ROTR(w14,61) ^ SHR(w14,6)) ||
           [(ROTR(w15,19) ^ ROTR(w15,61) ^ SHR(w15,6))](b0)__
           [(ROTR(w14,19) ^ ROTR(w14,61) ^ SHR(w14,6)) ||
           [(ROTR(w15,19) ^ ROTR(w15,61) ^ SHR(w15,6))](b1) =>
            [sigma_1_512(w14) || sigma_1_512(w15)](b0)__
            [sigma_1_512(w14) || sigma_1_512(w15)](b1) */
        return ROTR(x7, 19) ^ ROTR(x7, 61) ^ _mm256_srli_epi64(x7, 6);
    }
    static inline void sha512_update_x_avx2(__m256i x[8])
    {
        __m256i s0, s1;
        // Calculation of  sigma_0_512(x)+w0+w9 =
        // ROTR(x,1)^ROTR(x,8)^SHR(x,7)+w0+w9
        s0 = sigma_0_512(x[0], x[1], x[4], x[5]);
        // Calculation of sigma_1_512(x) = ROTR(x,19) ^ROTR(x,61)^SHR(x,6)
        s1   = sigma_1_512(x[7]);
        x[0] = _mm256_add_epi64(s0, s1);
        rotate_x(x);
    }

    static inline void extendMsgandCalcRound(
        __m256i& hash_256_0,
        __m256i& hash_256_1,
        __m256i  msg_vect[SHA512_CHUNK_NUM_VECT_AVX2 * 2],
        Uint64   ms[Sha512::cNumRounds + 16])
    {
        Uint32 k512_idx = 2 * 16;
        Uint64 a, b, c, d, e, f, g, h, temp;

        a = hash_256_0[0];

        b = hash_256_0[1];

        c = hash_256_0[2];

        d = hash_256_0[3];

        e = hash_256_1[0];

        f = hash_256_1[1];

        g = hash_256_1[2];

        h = hash_256_1[3];

        // Rounds 0-63 (0-15, 16-31, 32-47, 48-63)
        for (Uint32 i = 1; i < 5; i++) {

            UNROLL_8

            for (Uint32 j = 0; j < 8; j++) {
                const Uint32 pos = SHA512_WORDS_IN_128_BIT_VEC * j;
                ShaRound(a, b, c, d, e, f, g, h, ms[pos]);
                ShaRound(h, a, b, c, d, e, f, g, ms[pos + 1]);
                sha512_update_x_avx2(msg_vect);
                const __m256i y = _mm256_add_epi64(
                    msg_vect[7],
                    _mm256_loadu_si256(
                        (const __m256i*)&sha512_hash_constsx2[k512_idx]));

                _mm_store_si128((__m128i*)&(ms[16 + i * 16 + pos]),
                                _mm256_extracti128_si256(y, 1));
                _mm_store_si128((__m128i*)&(ms[pos]),
                                _mm256_extracti128_si256(y, 0));
                k512_idx += SHA512_WORDS_IN_256_BIT_VEC;

                temp = h;
                h    = f;
                f    = d;
                d    = b;
                b    = temp;
                temp = g;
                g    = e;
                e    = c;
                c    = a;
                a    = temp;
            }
        }

        // do 16 of the remaining for the first chunk
        ShaRound(a, b, c, d, e, f, g, h, ms[0]);
        ShaRound(h, a, b, c, d, e, f, g, ms[1]);
        ShaRound(g, h, a, b, c, d, e, f, ms[2]);
        ShaRound(f, g, h, a, b, c, d, e, ms[3]);
        ShaRound(e, f, g, h, a, b, c, d, ms[4]);
        ShaRound(d, e, f, g, h, a, b, c, ms[5]);
        ShaRound(c, d, e, f, g, h, a, b, ms[6]);
        ShaRound(b, c, d, e, f, g, h, a, ms[7]);
        ShaRound(a, b, c, d, e, f, g, h, ms[8]);
        ShaRound(h, a, b, c, d, e, f, g, ms[9]);
        ShaRound(g, h, a, b, c, d, e, f, ms[10]);
        ShaRound(f, g, h, a, b, c, d, e, ms[11]);
        ShaRound(e, f, g, h, a, b, c, d, ms[12]);
        ShaRound(d, e, f, g, h, a, b, c, ms[13]);
        ShaRound(c, d, e, f, g, h, a, b, ms[14]);
        ShaRound(b, c, d, e, f, g, h, a, ms[15]);

        hash_256_0 =
            _mm256_add_epi64(hash_256_0, _mm256_set_epi64x(d, c, b, a));
        hash_256_1 =
            _mm256_add_epi64(hash_256_1, _mm256_set_epi64x(h, g, f, e));

        a = hash_256_0[0];
        b = hash_256_0[1];
        c = hash_256_0[2];
        d = hash_256_0[3];
        e = hash_256_1[0];
        f = hash_256_1[1];
        g = hash_256_1[2];
        h = hash_256_1[3];

        UNROLL_80

        // do 80 rounds for the second chunk
        for (Uint32 i = 16; i < 96; i += 16) {
            ShaRound(a, b, c, d, e, f, g, h, ms[i]);
            ShaRound(h, a, b, c, d, e, f, g, ms[i + 1]);
            ShaRound(g, h, a, b, c, d, e, f, ms[i + 2]);
            ShaRound(f, g, h, a, b, c, d, e, ms[i + 3]);
            ShaRound(e, f, g, h, a, b, c, d, ms[i + 4]);
            ShaRound(d, e, f, g, h, a, b, c, ms[i + 5]);
            ShaRound(c, d, e, f, g, h, a, b, ms[i + 6]);
            ShaRound(b, c, d, e, f, g, h, a, ms[i + 7]);
            ShaRound(a, b, c, d, e, f, g, h, ms[i + 8]);
            ShaRound(h, a, b, c, d, e, f, g, ms[i + 9]);
            ShaRound(g, h, a, b, c, d, e, f, ms[i + 10]);
            ShaRound(f, g, h, a, b, c, d, e, ms[i + 11]);
            ShaRound(e, f, g, h, a, b, c, d, ms[i + 12]);
            ShaRound(d, e, f, g, h, a, b, c, ms[i + 13]);
            ShaRound(c, d, e, f, g, h, a, b, ms[i + 14]);
            ShaRound(b, c, d, e, f, g, h, a, ms[i + 15]);
        }

        hash_256_0 =
            _mm256_add_epi64(hash_256_0, _mm256_set_epi64x(d, c, b, a));
        hash_256_1 =
            _mm256_add_epi64(hash_256_1, _mm256_set_epi64x(h, g, f, e));
    }

    static inline void process_buffer_avx(Uint64       state[8],
                                          const Uint8* data,
                                          Uint32       length)
    {

        __attribute__((aligned(64))) Uint64 message_sch[Sha512::cNumRounds];
        __m128i                             msg_vect[SHA512_CHUNK_NUM_VECT_AVX];

        Uint64 a, b, c, d, e, f, g, h, t;
        a = state[0];
        b = state[1];
        c = state[2];
        d = state[3];
        e = state[4];
        f = state[5];
        g = state[6];
        h = state[7];

        // load_data
        static const __m128i shuf_mask =
            _mm_setr_epi32(0x04050607, 0x00010203, 0x0c0d0e0f, 0x08090a0b);

        UNROLL_8
        for (Uint32 j = 0; j < SHA512_CHUNK_NUM_VECT_AVX; j++) {
            const Uint32 pos = 2 * j;
            msg_vect[j] =
                _mm_loadu_si128((const __m128i*)(&data[sizeof(__m128i) * j]));
            msg_vect[j] = _mm_shuffle_epi8(msg_vect[j], shuf_mask);
            __m128i tmp = _mm_add_epi64(
                msg_vect[j],
                _mm_loadu_si128((const __m128i*)(&cRoundConstants[pos])));
            _mm_store_si128((__m128i*)(&message_sch[pos]), tmp);
        }

        // execute rounds 0 to 63
        Uint32 k512_idx = 16;
        for (Uint32 j = 0; j < 4; j++) {
            UNROLL_8
            for (Uint32 k = 0; k < SHA512_CHUNK_NUM_VECT_AVX; k++) {
                const Uint32 pos = 2 * k;

                __m128i temp[4];
                temp[0] = _mm_alignr_epi8(msg_vect[1], msg_vect[0], 8);

                temp[1] = _mm_srli_epi64(temp[0], 1);
                temp[2] = _mm_slli_epi64(temp[0], 64 - 1);
                temp[1] |= temp[2];

                temp[2] = _mm_srli_epi64(temp[0], 8);
                temp[3] = _mm_slli_epi64(temp[0], 64 - 8);
                temp[2] |= temp[3];

                temp[3] = _mm_srli_epi64(temp[0], 7);

                temp[0] = temp[1] ^ temp[2] ^ temp[3];

                temp[3] = _mm_alignr_epi8(msg_vect[5], msg_vect[4], 8);
                temp[3] = _mm_add_epi64(msg_vect[0], temp[3]);

                temp[0] = _mm_add_epi64(temp[0], temp[3]);

                // Calculation of s1
                temp[1] = _mm_srli_epi64(msg_vect[7], 19);
                temp[2] = _mm_slli_epi64(msg_vect[7], 64 - 19);
                temp[1] |= temp[2];

                temp[2] = _mm_srli_epi64(msg_vect[7], 61);
                temp[3] = _mm_slli_epi64(msg_vect[7], 64 - 61);
                temp[2] |= temp[3];

                temp[3] = _mm_srli_epi64(msg_vect[7], 6);
                temp[3] = temp[1] ^ temp[2] ^ temp[3];

                msg_vect[0] = _mm_add_epi64(temp[0], temp[3]);
                rotate_x(msg_vect);
                const __m128i y = _mm_add_epi64(
                    msg_vect[7],
                    _mm_loadu_si128(
                        (const __m128i*)(&cRoundConstants[k512_idx])));

                ShaRound(a, b, c, d, e, f, g, h, message_sch[pos]);
                ShaRound(h, a, b, c, d, e, f, g, message_sch[pos + 1]);

                t = h;
                h = f;
                f = d;
                d = b;
                b = t;
                t = g;
                g = e;
                e = c;
                c = a;
                a = t;
                _mm_store_si128((__m128i*)&message_sch[pos], y);
                k512_idx += 2;
            }
        }
        // do 16 of them
        ShaRound(a, b, c, d, e, f, g, h, message_sch[0]);
        ShaRound(h, a, b, c, d, e, f, g, message_sch[1]);
        ShaRound(g, h, a, b, c, d, e, f, message_sch[2]);
        ShaRound(f, g, h, a, b, c, d, e, message_sch[3]);
        ShaRound(e, f, g, h, a, b, c, d, message_sch[4]);
        ShaRound(d, e, f, g, h, a, b, c, message_sch[5]);
        ShaRound(c, d, e, f, g, h, a, b, message_sch[6]);
        ShaRound(b, c, d, e, f, g, h, a, message_sch[7]);
        ShaRound(a, b, c, d, e, f, g, h, message_sch[8]);
        ShaRound(h, a, b, c, d, e, f, g, message_sch[9]);
        ShaRound(g, h, a, b, c, d, e, f, message_sch[10]);
        ShaRound(f, g, h, a, b, c, d, e, message_sch[11]);
        ShaRound(e, f, g, h, a, b, c, d, message_sch[12]);
        ShaRound(d, e, f, g, h, a, b, c, message_sch[13]);
        ShaRound(c, d, e, f, g, h, a, b, message_sch[14]);
        ShaRound(b, c, d, e, f, g, h, a, message_sch[15]);
        // accumulate the state
        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
        state[5] += f;
        state[6] += g;
        state[7] += h;
    }
    static inline void
#if defined(COMPILER_IS_GCC)
        /* -fsched-stalled-insns=0 => This gcc compiler option optimizes the
         * pipeline as instructions are reordered considering the data stalls.
         * Zero argument means there is no limit on the number of instructions
         * which may be moved. Here the optimization is only applied
         * specifically for process_buffer_avx2 function as the same option for
         * process_buffer_avx reduces performance for the smaller block sizes
         * <=256
         */
        __attribute__((optimize("-fsched-stalled-insns=0")))
#endif
        process_buffer_avx2(Uint64       state[8],
                            const Uint8* data,
                            Uint32       num_chunks,
                            __m256i      hash_256_0,
                            __m256i      hash_256_1)
    {
        __attribute__((
            aligned(64))) Uint64 message_sch_1[Sha512::cNumRounds + 16];
        __m256i                  msg_vect[SHA512_CHUNK_NUM_VECT_AVX2 * 2]{};
        for (Uint32 i = 0; i < num_chunks; i = i + 2) {

            load_data(msg_vect,
                      message_sch_1,
                      /*&message_sch_1[80],*/ data + i * Sha512::cChunkSize);

            extendMsgandCalcRound(
                hash_256_0, hash_256_1, msg_vect, message_sch_1);
        }

        _mm256_storeu_si256((__m256i*)state, hash_256_0);

        _mm256_storeu_si256((__m256i*)(state + 4), hash_256_1);
    }

    alc_error_t ShaUpdate512(Uint64* pHash, const Uint8* pSrc, Uint64 src_len)
    {
        Uint32 num_chunks = src_len / Sha512::cChunkSize;
        if ((num_chunks & 0x01) == 1) {
            process_buffer_avx(pHash, pSrc, Sha512::cChunkSize);
            pSrc += Sha512::cChunkSize;
            num_chunks--;
        }
        __m256i hash_256_0, hash_256_1;
        auto    p_in_256 = reinterpret_cast<const __m256i*>(pHash);
        alcp_load_hash_ymm(p_in_256, hash_256_0, hash_256_1);
        process_buffer_avx2(pHash, pSrc, num_chunks, hash_256_0, hash_256_1);
        return ALC_ERROR_NONE;
    }
}} // namespace alcp::digest::zen4
