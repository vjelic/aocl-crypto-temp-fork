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

#include "alcp/ec.h"
#include "alcp/ecdh.h"

#include "alcp/alcp.hh"
#include "alcp/capi/defs.hh"
#include "alcp/capi/ec/builder.hh"
#include "alcp/capi/ec/ctx.hh"

using namespace alcp;

EXTERN_C_BEGIN

Uint64
alcp_ec_context_size(const alc_ec_info_p pEcInfo)
{
    Uint64 size = sizeof(ec::Context) + ec::EcBuilder::getSize(*pEcInfo);
#ifdef ALCP_ENABLE_DEBUG_LOGGING
    ALCP_DEBUG_LOG(LOG_DBG, "CtxSize %6ld", size);
#endif
    return size;
}

alc_error_t
alcp_ec_supported(const alc_ec_info_p pEcInfo)
{
#ifdef ALCP_ENABLE_DEBUG_LOGGING
    ALCP_DEBUG_LOG(LOG_INFO);
#endif
    alc_error_t err = ALC_ERROR_NONE;

    if (pEcInfo->ecCurveId != ALCP_EC_CURVE25519) {
        return ALC_ERROR_NOT_SUPPORTED;
    }

    if (pEcInfo->ecCurveType != ALCP_EC_CURVE_TYPE_MONTGOMERY) {
        return ALC_ERROR_NOT_SUPPORTED;
    }

    if (pEcInfo->ecPointFormat != ALCP_EC_POINT_FORMAT_UNCOMPRESSED) {
        return ALC_ERROR_NOT_SUPPORTED;
    }

    return err;
}

alc_error_t
alcp_ec_request(const alc_ec_info_p pEcInfo, alc_ec_handle_p pEcHandle)
{
#ifdef ALCP_ENABLE_DEBUG_LOGGING
    ALCP_DEBUG_LOG(LOG_INFO);
#endif
    alc_error_t err = ALC_ERROR_NONE;

    ALCP_BAD_PTR_ERR_RET(pEcHandle, err);
    ALCP_BAD_PTR_ERR_RET(pEcInfo, err);
    ALCP_BAD_PTR_ERR_RET(pEcHandle->context, err);

    auto ctx = static_cast<ec::Context*>(pEcHandle->context);

    new (ctx) ec::Context;

    ctx->status = ec::EcBuilder::Build(*pEcInfo, *ctx);

    return ctx->status.ok() ? err : ALC_ERROR_GENERIC;
}

alc_error_t
alcp_ec_set_privatekey(const alc_ec_handle_p pEcHandle,
                       const Uint8*          pPrivateKey)
{
#ifdef ALCP_ENABLE_DEBUG_LOGGING
    ALCP_DEBUG_LOG(LOG_INFO);
#endif
    alc_error_t err = ALC_ERROR_NONE;
    ALCP_BAD_PTR_ERR_RET(pEcHandle, err);
    ALCP_BAD_PTR_ERR_RET(pEcHandle->context, err);
    ALCP_BAD_PTR_ERR_RET(pPrivateKey,
                         err); // privateKey can be internal generated after
                               // adding DRBG and key managment function.
    auto ctx = static_cast<ec::Context*>(pEcHandle->context);

    ctx->status = ctx->setPrivateKey(ctx->m_ec, pPrivateKey);

    return ctx->status.ok() ? err : ALC_ERROR_GENERIC;
}

alc_error_t
alcp_ec_get_publickey(const alc_ec_handle_p pEcHandle,
                      Uint8*                pPublicKey,
                      const Uint8*          pPrivKey)
{
#ifdef ALCP_ENABLE_DEBUG_LOGGING
    ALCP_DEBUG_LOG(LOG_INFO);
#endif
    alc_error_t err = ALC_ERROR_NONE;
    ALCP_BAD_PTR_ERR_RET(pEcHandle, err);
    ALCP_BAD_PTR_ERR_RET(pEcHandle->context, err);
    ALCP_BAD_PTR_ERR_RET(pPublicKey, err);
    ALCP_BAD_PTR_ERR_RET(pPrivKey,
                         err); // privateKey can be internal generated after
                               // adding DRBG and key managment function.

    auto ctx = static_cast<ec::Context*>(pEcHandle->context);

    ctx->status = ctx->getPublicKey(ctx->m_ec, pPublicKey, pPrivKey);

    return ctx->status.ok() ? err : ALC_ERROR_GENERIC;
}

alc_error_t
alcp_ec_get_secretkey(const alc_ec_handle_p pEcHandle,
                      Uint8*                pSecretKey,
                      const Uint8*          pPublicKey,
                      Uint64*               pKeyLength)
{
#ifdef ALCP_ENABLE_DEBUG_LOGGING
    ALCP_DEBUG_LOG(LOG_INFO);
#endif
    alc_error_t err = ALC_ERROR_NONE;
    ALCP_BAD_PTR_ERR_RET(pEcHandle, err);
    ALCP_BAD_PTR_ERR_RET(pEcHandle->context, err);
    ALCP_BAD_PTR_ERR_RET(pSecretKey, err);
    ALCP_BAD_PTR_ERR_RET(pPublicKey, err);
    ALCP_BAD_PTR_ERR_RET(pKeyLength, err);

    auto ctx = static_cast<ec::Context*>(pEcHandle->context);

    ctx->status =
        ctx->getSecretKey(ctx->m_ec, pSecretKey, pPublicKey, pKeyLength);

    return ctx->status.ok() ? err : ALC_ERROR_GENERIC;
}

void
alcp_ec_finish(const alc_ec_handle_p pEcHandle)
{
#ifdef ALCP_ENABLE_DEBUG_LOGGING
    ALCP_DEBUG_LOG(LOG_INFO);
#endif
    auto ctx = static_cast<ec::Context*>(pEcHandle->context);

    /* TODO: fix the argument */
    ctx->status = ctx->finish(ctx->m_ec);

    ctx->~Context();

    // FIXME: Return error code if status is not ok
}

void
alcp_ec_reset(const alc_ec_handle_p pEcHandle)
{
#ifdef ALCP_ENABLE_DEBUG_LOGGING
    ALCP_DEBUG_LOG(LOG_INFO);
#endif
    auto ctx    = static_cast<ec::Context*>(pEcHandle->context);
    ctx->status = ctx->reset(ctx->m_ec);

    // FIXME: Return error code if status is not ok
}

EXTERN_C_END
