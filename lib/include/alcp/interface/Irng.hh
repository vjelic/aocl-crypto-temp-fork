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

#include "alcp/alcp.hh"
#include "alcp/error.h"

namespace alcp {

class ALCP_API_EXPORT IRng
{
  public:
    /**
     * TODO: Delete this in favour of randomize
     */
    virtual alc_error_t readRandom(Uint8* pBuf, Uint64 size) = 0;

    /**
     * Randomize a buffer array (byte-wise)
     *
     * May block if RNG is not initialized or not enough entropy available.
     * \param        output          Array of bytes
     * \param        length          length of output
     * \throws       RngNotSeeded    exception (if available)
     */
    virtual alc_error_t randomize(Uint8 output[], size_t length) = 0;

    /**
     * \return  Name of the RNG
     */
    virtual std::string name() const = 0;

    /**
     * Check if the RNG is seeded
     * \return  true if RNG is already seeded, false otherwise
     */

    virtual bool isSeeded() const = 0;

    /**
     * Helps to reseed the IRng
     * \return Bytes used to reseed
     */
    virtual size_t reseed() = 0;

    virtual alc_error_t setPredictionResistance(bool value) = 0;

  public:
    virtual ~IRng() = default;
};

} // namespace alcp
