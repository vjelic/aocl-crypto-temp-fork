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
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS!
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "alcp/base.hh"
#include "alcp/base/error.hh"
#include "alcp/capi/defs.hh"
#include "alcp/capi/mac/builder.hh"
#include "alcp/capi/mac/ctx.hh"
#include "alcp/mac.h"
#include "alcp/mac/mac.hh"

using namespace alcp;

EXTERN_C_BEGIN

Uint64
alcp_mac_context_size(void)
{
    Uint64 size = sizeof(mac::Context);
#ifdef ALCP_ENABLE_DEBUG_LOGGING
    ALCP_DEBUG_LOG(LOG_DBG, "CtxSize %6ld", size);
#endif
    return size;
}

alc_error_t
alcp_mac_request(alc_mac_handle_p pMacHandle, alc_mac_type_t mi_type)
{
#ifdef ALCP_ENABLE_DEBUG_LOGGING
    ALCP_DEBUG_LOG(LOG_INFO);
#endif
    alc_error_t err = ALC_ERROR_NONE;

    ALCP_BAD_PTR_ERR_RET(pMacHandle, err);
    ALCP_BAD_PTR_ERR_RET(pMacHandle->ch_context, err);

    auto p_ctx = static_cast<mac::Context*>(pMacHandle->ch_context);
    new (p_ctx) mac::Context;
    err = mac::MacBuilder::build(mi_type, p_ctx);

    return err;
}

alc_error_t
alcp_mac_update(alc_mac_handle_p pMacHandle, const Uint8* buff, Uint64 size)
{
#ifdef ALCP_ENABLE_DEBUG_LOGGING
    ALCP_DEBUG_LOG(LOG_DBG, "MacSize %6ld", size);
#endif
    alc_error_t err = ALC_ERROR_NONE;
    ALCP_BAD_PTR_ERR_RET(pMacHandle, err);
    ALCP_BAD_PTR_ERR_RET(pMacHandle->ch_context, err);

    auto p_ctx = static_cast<mac::Context*>(pMacHandle->ch_context);

    err = p_ctx->update(p_ctx->m_mac, buff, size);
    return err;
}

alc_error_t
alcp_mac_finalize(alc_mac_handle_p pMacHandle, Uint8* buff, Uint64 size)
{
#ifdef ALCP_ENABLE_DEBUG_LOGGING
    ALCP_DEBUG_LOG(LOG_DBG, "MacSize %6ld", size);
#endif
    alc_error_t err = ALC_ERROR_NONE;

    ALCP_BAD_PTR_ERR_RET(pMacHandle, err);
    ALCP_BAD_PTR_ERR_RET(pMacHandle->ch_context, err);

    auto p_ctx = static_cast<mac::Context*>(pMacHandle->ch_context);
    err        = p_ctx->finalize(p_ctx->m_mac, buff, size);
    return err;
}

alc_error_t
alcp_mac_finish(alc_mac_handle_p pMacHandle)
{
#ifdef ALCP_ENABLE_DEBUG_LOGGING
    ALCP_DEBUG_LOG(LOG_INFO);
#endif
    alc_error_t err = ALC_ERROR_NONE;

    ALCP_BAD_PTR_ERR_RET(pMacHandle, err);
    ALCP_BAD_PTR_ERR_RET(pMacHandle->ch_context, err);

    auto p_ctx = static_cast<mac::Context*>(pMacHandle->ch_context);
    p_ctx->finish(p_ctx->m_mac, p_ctx->m_digest);
    p_ctx->~Context();
    // FIXME: This function is always returning no errors
    return err;
}

alc_error_t
alcp_mac_reset(alc_mac_handle_p pMacHandle)
{
#ifdef ALCP_ENABLE_DEBUG_LOGGING
    ALCP_DEBUG_LOG(LOG_INFO);
#endif
    alc_error_t err = ALC_ERROR_NONE;

    ALCP_BAD_PTR_ERR_RET(pMacHandle, err);
    ALCP_BAD_PTR_ERR_RET(pMacHandle->ch_context, err);

    auto p_ctx = static_cast<mac::Context*>(pMacHandle->ch_context);
    err        = p_ctx->reset(p_ctx->m_mac);
    // FIXME: This function is always returning no errors
    return err;
}

alc_error_t
alcp_mac_init(alc_mac_handle_p pMacHandle,
              const Uint8*     key,
              Uint64           size,
              alc_mac_info_t*  info)
{
#ifdef ALCP_ENABLE_DEBUG_LOGGING
    ALCP_DEBUG_LOG(LOG_DBG, "KeySize %6ld", size);
#endif
    alc_error_t err = ALC_ERROR_NONE;

    ALCP_BAD_PTR_ERR_RET(pMacHandle, err);
    ALCP_BAD_PTR_ERR_RET(pMacHandle->ch_context, err);
    ALCP_BAD_PTR_ERR_RET(key, err);

    auto p_ctx = static_cast<mac::Context*>(pMacHandle->ch_context);
    err        = p_ctx->init(p_ctx, key, size, info);

    // FIXME: This function is always returning no errors
    return err;
}

alc_error_t
alcp_mac_context_copy(const alc_mac_handle_p pSrcHandle,
                      const alc_mac_handle_p pDestHandle)
{
#ifdef ALCP_ENABLE_DEBUG_LOGGING
    ALCP_DEBUG_LOG(LOG_INFO);
#endif
    alc_error_t err = ALC_ERROR_NONE;
    ALCP_BAD_PTR_ERR_RET(pSrcHandle, err);
    ALCP_BAD_PTR_ERR_RET(pDestHandle, err);

    auto src_ctx  = static_cast<mac::Context*>(pSrcHandle->ch_context);
    auto dest_ctx = static_cast<mac::Context*>(pDestHandle->ch_context);

    ALCP_BAD_PTR_ERR_RET(src_ctx, err);
    ALCP_BAD_PTR_ERR_RET(dest_ctx, err);

    new (dest_ctx) mac::Context;

    err = mac::MacBuilder::BuildWithCopy(src_ctx, dest_ctx);

    return err;
}
EXTERN_C_END