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

#include "aes/ipp_aes_init_common.hh"

IppStatus
ippsAES_CCMGetSize(int* pSize)
{
    printMsg("CCM GetSize");
    *pSize = sizeof(ipp_wrp_aes_aead_ctx);
    printMsg("CCM GetSize End");
    return ippStsNoErr;
}

IppStatus
ippsAES_CCMInit(const Ipp8u*      pKey,
                int               keyLen,
                IppsAES_CCMState* pState,
                int               ctxSize)
{
    printMsg("CCM Init");
    std::stringstream ss;
    ss << "KeyLength:" << keyLen;
    printMsg(ss.str());
    ipp_wrp_aes_ctx* context_cipher =
        &((reinterpret_cast<ipp_wrp_aes_aead_ctx*>(pState))->aead_ctx);
    if (pKey != nullptr) {
        context_cipher->key               = (Uint8*)pKey;
        context_cipher->keyLen            = keyLen * 8;
        context_cipher->mode              = ALC_AES_MODE_CCM;
        context_cipher->handle.ch_context = nullptr;
    } else {
        if (context_cipher->handle.ch_context != nullptr) {
            alcp_cipher_finish(&(context_cipher->handle));
            free(context_cipher->handle.ch_context);
            context_cipher->handle.ch_context = nullptr;
        }
    }
    (reinterpret_cast<ipp_wrp_aes_aead_ctx*>(pState))->msg_len = 0;
    (reinterpret_cast<ipp_wrp_aes_aead_ctx*>(pState))->tag_len = 0;
    printMsg("CCM Init End");
    return ippStsNoErr;
}
