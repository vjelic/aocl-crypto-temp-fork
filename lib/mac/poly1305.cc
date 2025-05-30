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
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS!
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "alcp/mac/poly1305.hh"
#include "alcp/base.hh"
#include "alcp/mac/poly1305_zen4.hh"
#include "alcp/utils/cpuid.hh"
#include <algorithm>
#include <array>
#include <iostream>
#include <tuple>
namespace alcp::mac::poly1305 {
using utils::CpuId;
template<utils::CpuArchFeature feature>
Poly1305<feature>::Poly1305()
{
    if constexpr (utils::CpuArchFeature::eReference == feature
                  || utils::CpuArchFeature::eAvx2 == feature) {
        poly1305_impl = std::make_unique<reference::Poly1305Ref>();
    } else if constexpr (utils::CpuArchFeature::eDynamic == feature) {
        // utils::CpuArchFeature::eDynamic
        if (!(CpuId::cpuHasAvx512(utils::Avx512Flags::AVX512_F)
              && CpuId::cpuHasAvx512(utils::Avx512Flags::AVX512_DQ)
              && CpuId::cpuHasAvx512(utils::Avx512Flags::AVX512_BW))) {
            poly1305_impl = std::make_unique<reference::Poly1305Ref>();
        }
    }
}
template<utils::CpuArchFeature feature>
Poly1305<feature>::Poly1305(const Poly1305& src)
{}

template<utils::CpuArchFeature feature>
alc_error_t
Poly1305<feature>::init(const Uint8 key[], Uint64 keyLen)
{
    alc_error_t err = ALC_ERROR_NONE;
    if (keyLen != 32) {
        std::cout << "ERROR KEYLEN:" << keyLen << std::endl;
        err = ALC_ERROR_NOT_SUPPORTED;
        return err;
    }
    state.finalized = false;
    if constexpr ((utils::CpuArchFeature::eReference == feature)
                  || (utils::CpuArchFeature::eAvx2 == feature)) {
        return poly1305_impl->init(key, keyLen);
    } else if constexpr (utils::CpuArchFeature::eAvx512 == feature) {
        zen4::poly1305_init_radix44(state, key);
        err = ALC_ERROR_NONE;
        return err;
    } else if constexpr (utils::CpuArchFeature::eDynamic == feature) {
        // Manual dispatch in case we don't know where to dispatch to.
        if (CpuId::cpuHasAvx512(utils::Avx512Flags::AVX512_F)
            && CpuId::cpuHasAvx512(utils::Avx512Flags::AVX512_DQ)
            && CpuId::cpuHasAvx512(utils::Avx512Flags::AVX512_BW)) {
            zen4::poly1305_init_radix44(state, key);
            err = ALC_ERROR_NONE;
            return err;
        } else {
            return poly1305_impl->init(key, keyLen);
        }
    }
    err = ALC_ERROR_BAD_STATE;
    return err;
}

template<utils::CpuArchFeature feature>
alc_error_t
Poly1305<feature>::update(const Uint8 pMsg[], Uint64 msgLen)
{
    alc_error_t err = ALC_ERROR_NONE;
    if constexpr ((utils::CpuArchFeature::eReference == feature)
                  || (utils::CpuArchFeature::eAvx2 == feature)) {
        return poly1305_impl->update(pMsg, msgLen);
    } else if constexpr (utils::CpuArchFeature::eAvx512 == feature) {
        if (zen4::poly1305_update_radix44(state, pMsg, msgLen) == true) {
            err = ALC_ERROR_NONE;
            return err;
        } else {
            err = ALC_ERROR_BAD_STATE;
            return err;
        }
    } else if constexpr (utils::CpuArchFeature::eDynamic == feature) {
        // Manual dispatch in case we don't know where to dispatch to.
        if (CpuId::cpuHasAvx512(utils::Avx512Flags::AVX512_F)
            && CpuId::cpuHasAvx512(utils::Avx512Flags::AVX512_DQ)
            && CpuId::cpuHasAvx512(utils::Avx512Flags::AVX512_BW)) {
            if (zen4::poly1305_update_radix44(state, pMsg, msgLen) == true) {
                err = ALC_ERROR_NONE;
                return err;
            } else {
                err = ALC_ERROR_BAD_STATE;
                return err;
            }
        } else {
            return poly1305_impl->update(pMsg, msgLen);
        }
    }
    err = ALC_ERROR_BAD_STATE;
    return err;
}

template<utils::CpuArchFeature feature>
alc_error_t
Poly1305<feature>::reset()
{
    alc_error_t err = ALC_ERROR_NONE;
    if constexpr ((utils::CpuArchFeature::eReference == feature)
                  || (utils::CpuArchFeature::eAvx2 == feature)) {
        return poly1305_impl->reset();
    } else if constexpr (utils::CpuArchFeature::eAvx512 == feature) {
        state.reset();
        err = ALC_ERROR_NONE;
        return err;
    } else if constexpr (utils::CpuArchFeature::eDynamic == feature) {
        // Manual dispatch in case we don't know where to dispatch to.
        if (CpuId::cpuHasAvx512(utils::Avx512Flags::AVX512_F)
            && CpuId::cpuHasAvx512(utils::Avx512Flags::AVX512_DQ)
            && CpuId::cpuHasAvx512(utils::Avx512Flags::AVX512_BW)) {
            state.reset();
            err = ALC_ERROR_NONE;
            return err;
        } else {
            return poly1305_impl->reset();
        }
    }
    err = ALC_ERROR_BAD_STATE;
    return err;
}

template<utils::CpuArchFeature feature>
alc_error_t
Poly1305<feature>::finalize(Uint8 digest[], Uint64 digestLen)
{
    alc_error_t err = ALC_ERROR_NONE;
    if constexpr ((utils::CpuArchFeature::eReference == feature)
                  || (utils::CpuArchFeature::eAvx2 == feature)) {
        return poly1305_impl->finish(digest, digestLen);
    } else if constexpr (utils::CpuArchFeature::eAvx512 == feature) {
        if (zen4::poly1305_finalize_radix44(state, digest, digestLen) == true) {
            err = ALC_ERROR_NONE;
            return err;
        } else {
            err = ALC_ERROR_BAD_STATE;
            return err;
        }
    } else if constexpr (utils::CpuArchFeature::eDynamic == feature) {
        // Manual dispatch in case we don't know where to dispatch to.
        if (CpuId::cpuHasAvx512(utils::Avx512Flags::AVX512_F)
            && CpuId::cpuHasAvx512(utils::Avx512Flags::AVX512_DQ)
            && CpuId::cpuHasAvx512(utils::Avx512Flags::AVX512_BW)) {
            if (zen4::poly1305_finalize_radix44(state, digest, digestLen)
                == true) {
                err = ALC_ERROR_NONE;
                return err;
            } else {
                err = ALC_ERROR_BAD_STATE;
                return err;
            }
        } else {
            return poly1305_impl->finish(digest, digestLen);
        }
    }
    err = ALC_ERROR_BAD_STATE;
    return err;
}
template class Poly1305<utils::CpuArchFeature::eAvx512>;
template class Poly1305<utils::CpuArchFeature::eAvx2>;
template class Poly1305<utils::CpuArchFeature::eReference>;
template class Poly1305<utils::CpuArchFeature::eDynamic>;
} // namespace alcp::mac::poly1305