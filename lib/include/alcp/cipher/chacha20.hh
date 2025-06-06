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

#include "alcp/base.hh"
#include "alcp/cipher.h"
#include "alcp/utils/cpuid.hh"

#include "alcp/cipher/cipher_common.hh"

namespace alcp::cipher {
using utils::CpuCipherFeatures;
using utils::CpuId;

static constexpr Uint32 Chacha20Constants[4] = {
    0x61707865, 0x3320646e, 0x79622d32, 0x6b206574
};

#define CHACHA20_BLOCK_SIZE 64

#define CHACHA_CLASS_GEN(CHILD_NEW, PARENT)                                    \
    class ALCP_API_EXPORT CHILD_NEW : public PARENT                            \
                                                                               \
    {                                                                          \
      public:                                                                  \
        /* CHILD_NEW(alc_cipher_data_t* ctx)                                   \
            : PARENT(ctx){};*/                                                 \
        ~CHILD_NEW(){};                                                        \
                                                                               \
      public:                                                                  \
        alc_error_t encrypt(const Uint8* pInput,                               \
                            Uint8*       pOutput,                              \
                            Uint64       pInputLen);                                 \
        alc_error_t decrypt(const Uint8* pInput,                               \
                            Uint8*       pOutput,                              \
                            Uint64       pInputLen);                                 \
    };

class ALCP_API_EXPORT ChaCha20 : public virtual iCipher
{
    // ChaCha256 should be able to access this
  protected:
    static constexpr Uint64 cMKeylen    = 256 / 8;
    static constexpr Uint64 cMIvlen     = (128 / 8);
    static constexpr int    cMBlockSize = CHACHA20_BLOCK_SIZE;
    alignas(16) Uint8 m_key[cMKeylen];

  protected:
    alignas(16) Uint8 m_iv[cMIvlen];

    // FIXME: Needs to be private or protected after chacha20-poly1305
    // integration
  public:
    alc_error_t setKey(const Uint8 key[], Uint64 keylen);
    alc_error_t setIv(const Uint8 iv[], Uint64 ivlen);

  private:
    static alc_error_t validateKey(const Uint8* key, Uint64 keylen);
    static alc_error_t validateIv(const Uint8 iv[], Uint64 iVlen);

  public:
    ChaCha20(Uint32 keyLen_in_bytes){};
    alc_error_t init(const Uint8* pKey,
                     const Uint64 keyLen,
                     const Uint8* pIv,
                     const Uint64 ivLen) override;
};

namespace vaes512 {
    CIPHER_CLASS_GEN_(ChaCha256, ChaCha20, virtual iCipher, 256 / 8)

} // namespace vaes512

namespace ref {
    CIPHER_CLASS_GEN_(ChaCha256, ChaCha20, virtual iCipher, 256 / 8)
} // namespace ref

} // namespace alcp::cipher
