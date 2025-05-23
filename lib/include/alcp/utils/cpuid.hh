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

#pragma once

#include "alcp/alcp.hh"
#include "config.h"
#include <iostream>
#include <memory>

namespace alcp::utils {

enum class CpuCipherFeatures
{
    eReference = 0,
    eAesni     = 1,
    eVaes256   = 2,
    eVaes512   = 3,
    eDynamic   = 4
};

enum class CpuArchFeature
{
    eReference = 0,
    eAvx2      = 2,
    eAvx512    = 3,
    eDynamic   = 4
};

enum class Avx512Flags
{
    AVX512_DQ = 1,
    AVX512_F,
    AVX512_BW,
    AVX512_IFMA,
    AVX512_VL,
};

enum class CpuZenVer
{
    ZEN  = 0,
    ZEN2 = 1,
    ZEN3 = 2,
    ZEN4 = 3,
    ZEN5 = 4,
};

// using alci::Cpu;
// using alci::Uarch;

class ALCP_API_EXPORT CpuId
{
  public:
    CpuId() {}
    ~CpuId() = default;

    // Genoa functions
    /**
     * @brief Returns true if CPU has AVX512f Flag
     *
     * @return true
     * @return false
     */
    static bool cpuHasAvx512f();
    /**
     * @brief Returns true if CPU has AVX512DQ Flag
     *
     * @return true
     * @return false
     */
    static bool cpuHasAvx512dq();
    /**
     * @brief Retrurns true if CPU has AVX512BW Flag
     *
     * @return true
     * @return false
     */
    static bool cpuHasAvx512bw();
    /**
     * @brief Retrurns true if CPU has AVX512IFMA Flag
     *
     * @return true
     * @return false
     */
    static bool cpuHasAvx512ifma();
    /**
     * @brief Retrurns true if CPU has AVX512VL Flag
     *
     * @return true
     * @return false
     */
    static bool cpuHasAvx512vl();
    /**
     * @brief Returns true depending on the flag is available or not on CPU
     *
     * @param flag
     * @return true
     * @return false
     */
    static bool cpuHasAvx512(Avx512Flags flag);

    // Milan functions
    /**
     * @brief Returns true if CPU supports vector AES
     * @note  Will return true if either 256 or 512 bit vector AES is supported
     *
     * @return true
     * @return false
     */
    static bool cpuHasVaes();

    // Rome functions
    /**
     * @brief Returns true if CPU supports block AES instruction
     *
     * @return true
     * @return false
     */
    static bool cpuHasAesni();
    /**
     * @brief Returns true if CPU supports block SHA instruction
     *
     * @return true
     * @return false
     */
    static bool cpuHasShani();
    /**
     * @brief Returns true if CPU supports AVX2 instructions
     *
     * @return true
     * @return false
     */
    static bool cpuHasAvx2();
    /**
     * @brief Returns true if CPU supports SSE3 instructions
     *
     * @return true
     * @return false
     */
    static bool cpuHasSse3();
    /**
     * @brief Returns true if RDRAND, secure RNG number generator is supported
     * by CPU
     *
     * @return true
     * @return false
     */
    static bool cpuHasRdRand();
    /**
     * @brief Returns true if ADX is supported
     * by CPU
     *
     * @return true
     * @return false
     */
    static bool cpuHasAdx();
    /**
     * @brief Returns true if BMI2 is supported
     * by CPU
     *
     * @return true
     * @return false
     */
    static bool cpuHasBmi2();
    /**
     * @brief Returns true if RDSEED, secure RNG seed generator is supported by
     * CPU
     *
     * @return true
     * @return false
     */
    static bool cpuHasRdSeed();
    /**
     * @brief Returns true if currently executing cpu is Zen1
     *
     * @return true
     * @return false
     */
    static bool cpuIsZen1();
    /**
     * @brief Returns true if currently executing cpu is Zen2
     *
     * @return true
     * @return false
     */
    static bool cpuIsZen2();
    /**
     * @brief Returns true if currently executing cpu is Zen3
     *
     * @return true
     * @return false
     */
    static bool cpuIsZen3();
    /**
     * @brief Returns true if currently executing cpu is Zen4
     *
     * @return true
     * @return false
     */
    static bool cpuIsZen4();
    /**
     * @brief Returns true if currently executing cpu is Zen5
     *
     * @return true
     * @return false
     */
    static bool cpuIsZen5();

  private:
    class Impl;

  public:
    static std::unique_ptr<Impl> pImpl;
};
} // namespace alcp::utils