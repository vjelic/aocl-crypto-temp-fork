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
 *-
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS!
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once
#include "alcp/base.hh"
#include "alcp/mac/mac.hh"
#include "alcp/mac/poly1305-ref.hh"
#include "alcp/mac/poly1305_state.hh"
#include "alcp/utils/cpuid.hh"

#define POLY1305_RADIX_26 false

namespace alcp::mac::poly1305 {
using utils::CpuArchFeature;
template<utils::CpuArchFeature feature = CpuArchFeature::eDynamic>
class ALCP_API_EXPORT Poly1305 : public IMac
{
  private:
    std::unique_ptr<reference::Poly1305Ref> poly1305_impl;
    Poly1305State44                         state;

  public:
    /**
     * @brief Given message, updates internal state processing the message
     * @param pMsg  Byte addressible message
     * @param msgLen  Length of message in bytes
     * @return  alc_error_t/Result of the operation
     */
    alc_error_t update(const Uint8 pMsg[], Uint64 msgLen) override;
    /**
     * @brief Sets the Key and Initializes the state of Poly1305
     * @param key - Key to use for Poly1305
     * @param keyLen - Key Length 32 Byte, anything else wont work
     * @return alc_error_t/Result of the operation
     */
    alc_error_t init(const Uint8 key[], Uint64 keyLen);
    /**
     * @brief Resets the temporary buffers without clearing key
     * @return alc_error_t/Result of the operation
     */
    alc_error_t reset() override;
    /**
     * @brief
     * @param digest mac buffer
     * @param digestLen Length of mac in bytes
     * @return alc_error_t/Result of the operation
     */
    alc_error_t finalize(Uint8 digest[], Uint64 digestLen) override;

    Poly1305();
    virtual ~Poly1305() = default;
    Poly1305(const Poly1305& src);
};
} // namespace alcp::mac::poly1305