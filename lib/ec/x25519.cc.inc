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

#include <immintrin.h>
#include <string.h>

#include "alcp/ec/ecdh.hh"
#include "alcp/ec/ed_25519_table.hh"
#include "alcp/ec/x25519_radix64bit.hh"
#include "alcp/utils/copy.hh"

using namespace radix64bit;

static inline void
FetchIntermediateMul(Int8 i, Int8 j, PrecomputedPoint& point);

static inline void
MontLadder(Uint64       resultx[4],
           Uint64       resultz[4],
           const Uint8* pScalar,
           const Uint64 basePoint[4]);

static inline void
InverseX25519(Uint64 out[4], const Uint64 in[4])
{

    Uint64 a[4], b[4], c[4], d[4];

    // square and Multiply algorithm to compute z ^ (2^5 * (2^250-1) + 11)

    SquareX25519Count(a, in, 1); // a = z^2
    SquareX25519Count(d, a, 2);  // d = z^8
    MulX25519(b, d, in);         // b = z^8*z     = z^9
    MulX25519(a, b, a);          // a = z^9*z^2   = z^11
    SquareX25519Count(d, a, 1);  // d = sqr(z^11) = z^22

    /*
     * b  = z^22 * z^9
     *    = z^31 = z ^(32-1)
     *    = z^ (2^5 - 1)
     */
    MulX25519(b, d, b);

    /*
     * d = sqr5times(z^ (2^5 - 1))
     *   = z ^ (2^10 - 2^5)
     */
    SquareX25519Count(d, b, 5);

    /*
     * b = z ^ (2^10 - 2^5) * z^ (2^5 - 1)
     *   = z ^ (2^10 - 2^5 + 2^5 - 1)
     *   = z ^ (2 ^10 - 1 )
     *   = z ^ (2 ^10 - 2^0 )
     */
    MulX25519(b, d, b);

    /*
     * d = sqr10times(  z ^ (2^10 - 2^5) )
     *   = z ^ (2^20 - 2^10)
     */
    SquareX25519Count(d, b, 10);

    /*
     * c = z ^ (2^20 - 2^10) *  z ^ (2 ^10 - 2^0 )
     *   = z ^ (2^20 - 2^0)
     */
    MulX25519(c, d, b);

    /*
     *  c = sqr20times( z ^ (2^20 - 2^10) )
     *    = z ^ (2^40 - 2^20)
     */
    SquareX25519Count(d, c, 20);
    MulX25519(d, d, c);            /* d = z ^ (2^40 - 2^0)   */
    SquareX25519Count(d, d, 10);   /* d = z ^ (2^50 - 2^10)  */
    MulX25519(b, d, b);            /* b = z ^ (2^50 - 2^0)   */
    SquareX25519Count(d, b, 50);   /* d = z ^ (2^100 - 2^50) */
    MulX25519(c, d, b);            /* c = z ^ (2^100 - 2^0)  */
    SquareX25519Count(d, c, 100);  /* d = z ^ (2^200 - 2^100)*/
    MulX25519(d, d, c);            /* d = z ^ (2^200 - 2^0)  */
    SquareX25519Count(d, d, 50);   /* d = z ^ (2^250 - 2^50) */
    MulX25519(d, d, b);            /* d = z ^ (2^250 - 2^0)  */
    SquareX25519Count(d, d, 5);    /* d = z ^ (2^255 - 2^5)  */
    MulX25519((Uint64*)out, d, a); /* Inverse(z) = z ^ (2^255 - 21)   */
}

static inline void
PointAddInEdward(AlcpEcPointExtended& result, const PrecomputedPoint& point)
{
    // a ← (y1 − x1) · (y2 − x2), b ← (y1 + x1) · (y2 + x2), c ← k t1 · t2,
    // k <-2dconst, d ← 2z1 · z2, e ← b − a, f ← d − c, g ← d + c, h ← b + a, x3
    // ← e · f, y3 ← g · h, t3 ← e · h, z3 ← f · g

    Uint64 a[4], b[4], c[4], d[4], e[4], f[4], g[4];
    SubX25519(a, result.y, result.x);
    MulX25519(a, a, point.m_y);
    SumX25519(b, result.y, result.x);
    MulX25519(b, b, point.m_x);
    MulX25519(c, result.t, point.m_z);
    SumX25519(d, result.z, result.z);
    SubX25519(e, b, a);
    SumX25519(b, b, a);

    SubX25519(f, d, c);
    SumX25519(g, d, c);

    MulX25519(result.x, e, f);
    MulX25519(result.y, g, b);
    MulX25519(result.z, f, g);
    MulX25519(result.t, e, b);
}

static inline void
CovertEdwardToMont(const AlcpEcPointExtended& result, Uint8* pPublicKey)
{
    Uint64 numerator[4], denominator[4], inverse_denominator[4];

    SumX25519(numerator, result.z, result.y);
    SubX25519(denominator, result.z, result.y);
    InverseX25519(inverse_denominator, denominator);
    MulX25519(numerator, numerator, inverse_denominator);
    RadixToBytes(reinterpret_cast<Uint64*>(pPublicKey), numerator);
}

static void
MontCore(Uint64*       x2, // output x2
         Uint64*       z2, // output z2
         Uint64*       x3, // output x3
         Uint64*       z3, // output z3
         Uint64*       a,  // input x
         Uint64*       b,  // input z
         Uint64*       c,  // input xp
         Uint64*       d,  // input zp
         const Uint64* basePoint)
{
    Uint64 t1[4], t2[4], t3[4], t[4], a2[4], c2[4], m[4], n[4], p[4];

    SumX25519(t1, a, b);
    SubX25519(b, a, b);

    SumX25519(t2, c, d);
    SubX25519(d, c, d);

    // to do: parallel mul once r*19 is reducted to 52 bits
    MulX25519(m, t2, b);
    MulX25519(n, t1, d);

    SumX25519(t3, m, n);
    SubX25519(n, m, n);

    // to do, parallel square
    SquareX25519Count(x3, t3, 1);
    SquareX25519Count(p, n, 1);
    SquareX25519Count(a2, t1, 1);
    SquareX25519Count(c2, b, 1);

    MulX25519(z3, p, basePoint);

    MulX25519(x2, a2, c2);
    SubX25519(c2, a2, c2);

    ScalarMulX25519(t, c2);

    SumX25519(t, t, a2);
    MulX25519(z2, c2, t);
}

void
AlcpScalarPubX25519(Int8* privKeyRadix32, Uint8* pPublicKey)
{

    AlcpEcPointExtended result;

    PrecomputedPoint point;
    UNROLL_52
    for (int i = 51; i >= 0; i--) {
        FetchIntermediateMul(i, privKeyRadix32[i], point);
        PointAddInEdward(result, point);
    }
    CovertEdwardToMont(result, pPublicKey);
}

void
alcpScalarMulX25519(Uint8* secret, const Uint8* scalar, const Uint8* basepoint)
{
    Uint64 bp[4], x[4] = { 0 }, z[4] = { 0 }, zInverse[4];

    BytesToRadix(bp, (Uint64*)basepoint);

    MontLadder(x, z, scalar, bp);

    InverseX25519(zInverse, z);
    MulX25519(z, x, zInverse);

    RadixToBytes((Uint64*)secret, z);
}
