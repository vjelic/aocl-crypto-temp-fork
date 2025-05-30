/*
 * Copyright (C) 2023-2025, Advanced Micro Devices. All rights reserved.
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

#ifndef _CIPHER_AES_HH_
#define _CIPHER_AES_HH_ 2

#include "alcp/cipher.h"
#include "alcp/cipher.hh"

#include "alcp/base.hh"
#include "alcp/cipher/rijndael.hh"
#include "alcp/utils/bits.hh"
#include "alcp/utils/memory.hh"

#include <immintrin.h>
#include <wmmintrin.h>

namespace alcp::cipher {

#define ALCP_ENC           1
#define ALCP_DEC           0
#define MAX_CIPHER_IV_SIZE (1024 / 8)

typedef struct alc_cipher_key_data
{
    // key expanded
    const Uint8* m_enc_key;
    const Uint8* m_dec_key;
} alc_cipher_key_data_t;

/*
 * @brief       AES (Advanced Encryption Standard)
 *
 * @note       AES is currently same as Rijndael, This may be renamed to
 *              other as well in the future.
 *
 * TODO: We need to move the exception to an init() function. as the constructor
 * is notes fully complete, and exception would cause destructor to be called on
 * object that is not fully constructed
 */
class Aes : public Rijndael
{
  public:
    Uint32                m_nrounds = 0;
    alc_cipher_key_data_t m_cipher_key_data{};

    Uint32                             m_keyLen_in_bytes_aes        = 0;
    __attribute__((aligned(16))) Uint8 m_iv_aes[MAX_CIPHER_IV_SIZE] = {};
    Uint8*                             m_pIv_aes                    = m_iv_aes;
    Uint64                             m_ivLen_aes                  = 0;
    Uint32                             m_isKeySet_aes               = 0;
    Uint32                             m_ivState_aes                = 0;
    Uint32                             m_isEnc_aes                  = ALCP_ENC;
    Uint64                             m_dataLen                    = 0;

    // Data Size Limits
    Uint32 m_ivLen_max = MAX_CIPHER_IV_SIZE;
    Uint32 m_ivLen_min = 1;

    Aes(Uint32 keyLen_in_bytes)
        : Rijndael()
    {
        m_keyLen_in_bytes_aes = keyLen_in_bytes;
        utils::memlock(m_iv_aes, MAX_CIPHER_IV_SIZE);
    }

    // this constructor to be removed.
    Aes()
        : Rijndael()
    {
        utils::memlock(m_iv_aes, MAX_CIPHER_IV_SIZE);
    }

  protected:
    virtual ~Aes()
    {
        utils::memunlock(m_iv_aes, MAX_CIPHER_IV_SIZE);
        std::fill(m_iv_aes, m_iv_aes + MAX_CIPHER_IV_SIZE, 0);
    }

    // FIXME:
    // Without CMAC-SIV extending AES, we cannot access it with protected,
    // Please change to protected if needed in future
  public:
    ALCP_API_EXPORT alc_error_t init(const Uint8* pKey,
                                     const Uint64 keyLen,
                                     const Uint8* pIv,
                                     const Uint64 ivLen);

    static bool isSupported(const Uint32 keyLen)
    {
        if ((keyLen == ALC_KEY_LEN_128) || (keyLen == ALC_KEY_LEN_192)
            || (keyLen == ALC_KEY_LEN_256)) {
            return true;
        }
        return false;
    }

    alc_error_t setKey(const Uint8* pKey, Uint8* pExpKey, const Uint64 keyLen);
    alc_error_t setKey(const Uint8* pKey, const Uint64 keyLen);
    alc_error_t setIv(const Uint8* pIv, const Uint64 ivLen);

    void getKey()
    {
        m_cipher_key_data.m_enc_key = getEncryptKeys();
        m_cipher_key_data.m_dec_key = getDecryptKeys();
        m_nrounds                   = getRounds();
    }

  protected:
    ALCP_API_EXPORT virtual alc_error_t setMode(CipherMode mode);

  protected:
    CipherMode m_mode{};
    void*      m_this{};
};

} // namespace alcp::cipher

#endif /* _CIPHER_AES_HH_ */
