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

#include "alcp/error.h"
#include "cmac.hh"
#include <type_traits> /* for is_same_v<> */

namespace alcp::mac {
using namespace alcp::base::status;

class CmacBuilder
{
  public:
    static alc_error_t build(Context* ctx);
};

static alc_error_t
__cmac_wrapperInit(Context*        ctx,
                   const Uint8*    key,
                   Uint64          size,
                   alc_mac_info_t* info)
{
    alc_error_t err    = ALC_ERROR_NONE;
    auto        p_cmac = static_cast<Cmac*>(ctx->m_mac);
    p_cmac->init(key, size); // p_cmac->init(key, size, NULL, 0);
    return err;
}

static alc_error_t
__cmac_wrapperUpdate(void* cmac, const Uint8* buff, Uint64 size)
{
    auto p_cmac = static_cast<Cmac*>(cmac);
    return p_cmac->update(buff, size);
}

static alc_error_t
__cmac_wrapperFinalize(void* cmac, Uint8* buff, Uint64 size)
{
    auto p_cmac = static_cast<Cmac*>(cmac);
    return p_cmac->finalize(buff, size);
}

static void
__cmac_wrapperFinish(void* cmac, void* digest)
{
    auto p_cmac = static_cast<Cmac*>(cmac);
    delete p_cmac;
    // Not deleting the memory because it is allocated by application
}

static alc_error_t
__cmac_wrapperReset(void* cmac)
{
    auto p_cmac = static_cast<Cmac*>(cmac);
    return p_cmac->reset();
}

static alc_error_t
__cmac_build_with_copy(Context* srcCtx, Context* destCtx)
{
    auto cmac = new Cmac(*reinterpret_cast<Cmac*>(srcCtx->m_mac));

    destCtx->m_mac = static_cast<void*>(cmac);

    destCtx->init      = srcCtx->init;
    destCtx->update    = srcCtx->update;
    destCtx->finalize  = srcCtx->finalize;
    destCtx->finish    = srcCtx->finish;
    destCtx->reset     = srcCtx->reset;
    destCtx->duplicate = srcCtx->duplicate;
    return ALC_ERROR_NONE;
}

alc_error_t
CmacBuilder::build(Context* ctx)
{
    alc_error_t err{ ALC_ERROR_NONE };
    auto        p_algo = new Cmac();

    if (p_algo == nullptr) {
        // Unable to Allocate Memory for CMAC Object
        return ALC_ERROR_NO_MEMORY;
    }
    ctx->m_mac     = static_cast<void*>(p_algo);
    ctx->init      = __cmac_wrapperInit;
    ctx->update    = __cmac_wrapperUpdate;
    ctx->finalize  = __cmac_wrapperFinalize;
    ctx->finish    = __cmac_wrapperFinish;
    ctx->reset     = __cmac_wrapperReset;
    ctx->duplicate = __cmac_build_with_copy;
    return err;
}

} // namespace alcp::mac