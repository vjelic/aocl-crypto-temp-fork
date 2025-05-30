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

#include "common/context.hh"
#include "common/debug.hh"
#include "common/error.hh"
#include <alcp/alcp.h>
#include <alcp/types.h>
#include <iostream>
#include <ippcp.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief Encrypt Decrypt Common Function for CBC,CTR,CFB,OFB and XTS
 *
 * @param pSrc            Source (Ciphertext during dec else Plaintext)
 * @param pDst            Destination (Plaintext during dec else Ciphertext)
 * @param len             Length of Source
 * @param pCtx            Context of Wrapper
 * @param pCtrValue       Counter/IV value
 * @param ctrNumBitSize   Unused, ignore
 * @param mode            ALCP Mode
 * @param enc             True means encrypt mode
 * @return IppStatus      NotSupported,Error
 */
inline IppStatus
alcp_encdecAES(const Ipp8u*       pSrc,
               Ipp8u*             pDst,
               int                len,
               const IppsAESSpec* pCtx,
               const Ipp8u*       pCtrValue,
               int                ctrNumBitSize,
               alc_cipher_mode_t  mode,
               bool               enc)
{
    // Should replace below with something better as it does discard const
    ipp_wrp_aes_ctx* context = (ipp_wrp_aes_ctx*)(pCtx);
    alc_error_t      err;

    /* Continue initialization as we didnt have iv in initialization function
       if we already have context then it's already good, we can take it as
       already initialized. */

    if (context->handle.ch_context == nullptr) {
        context->mode = mode;
        context->iv   = (Uint8*)pCtrValue;

        auto size                  = alcp_cipher_context_size();
        context->handle.ch_context = (alc_cipher_context_p)(malloc(size));

// TODO: Debug statements, remove once done.
// Leaving debug statements here as XTS testing framework needs to be debugged.
#ifdef DEBUG
        if (mode == ALC_AES_MODE_XTS) {
            std::cout << "MODE:XTS" << std::endl;
            std::cout << "KEY:"
                      << parseBytesToHexStr(context->key, (context->keyLen) / 8)
                      << std::endl;
            std::cout << "KEYLen:" << context->keyLen / 8 << std::endl;
            std::cout << "TKEY:"
                      << parseBytesToHexStr(context->key
                                                + ((context->keyLen) / 8),
                                            ((context->keyLen) / 8))
                      << std::endl;
            std::cout << "KEYLen:" << context->keyLen / 8 << std::endl;
            std::cout << "IV:" << parseBytesToHexStr(context->iv, 16)
                      << std::endl;
            std::cout << "INLEN:" << len << std::endl;
            std::cout << "IN:"
                      << parseBytesToHexStr(
                             reinterpret_cast<const Uint8*>(pSrc), len)
                      << std::endl;
        }
#endif
        err = alcp_cipher_request(mode, context->keyLen, &(context->handle));
        if (alcp_is_error(err)) {
            printErr("unable to request");
            free(context->handle.ch_context);
            context->handle.ch_context = nullptr;
            return ippStsErr;
        }
    }

    // init
    err = alcp_cipher_init(&(context->handle),
                           context->key,
                           context->keyLen,
                           context->iv,
                           16); // FIXME: 16 should be variable
    if (alcp_is_error(err)) {
        printErr("Error in init\n");
    }

    // Do the actual decryption
    if (enc) {
        // err = alcp_cipher_encrypt(&handle, plaintxt, ciphertxt, len, iv);
        err = alcp_cipher_encrypt(&(context->handle),
                                  reinterpret_cast<const Uint8*>(pSrc),
                                  reinterpret_cast<Uint8*>(pDst),
                                  len);
    } else {
        err = alcp_cipher_decrypt(&(context->handle),
                                  reinterpret_cast<const Uint8*>(pSrc),
                                  reinterpret_cast<Uint8*>(pDst),
                                  len);
    }
#ifdef DEBUG
    if (mode == ALC_AES_MODE_XTS) {
        std::cout << "OUT:"
                  << parseBytesToHexStr(reinterpret_cast<const Uint8*>(pDst),
                                        len)
                  << std::endl;
    }
#endif
    // Messup ciphertext to test wrapper
    // *(reinterpret_cast<Uint8*>(pDst)) = 0x00;

    if (alcp_is_error(err)) {
        printErr("Unable decrypt");
        return ippStsUnderRunErr;
    }
    printMsg("Decrypt succeeded");
#ifdef DEBUG
    if (mode == ALC_AES_MODE_XTS) {
        std::cout << std::endl;
    }
#endif
    alcp_cipher_finish(&context->handle);
    if (context->handle.ch_context) {
        free(context->handle.ch_context);
        context->handle.ch_context = nullptr;
    }
    /*At this point it should be supported and alcp context should exist*/
    return ippStsNoErr;
}