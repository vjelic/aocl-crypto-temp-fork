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

#include "cmac/openssl_cmac.hh"
#include <cstddef>

namespace alcp::testing {

OpenSSLCmacBase::~OpenSSLCmacBase()
{
    if (m_handle != nullptr) {
        EVP_MAC_CTX_free(m_handle);
    }
    if (m_mac != nullptr) {
        EVP_MAC_free(m_mac);
    }
}

bool
OpenSSLCmacBase::Init(const alc_mac_info_t& info, std::vector<Uint8>& Key)
{
    m_info    = info;
    m_key     = &Key[0];
    m_key_len = Key.size();

    OSSL_PARAM  params[3] = {};
    size_t      params_n  = 0;
    const char* cipher    = NULL;

    switch (m_key_len * 8) {
        case 128:
            cipher = "aes-128-cbc";
            break;
        case 192:
            cipher = "aes-192-cbc";
            break;
        case 256:
            cipher = "aes-256-cbc";
            break;
        default:
            std::cout << "Error! invalid/unsupported keysize" << std::endl;
            break;
    }

    if (m_mac != nullptr) {
        EVP_MAC_free(m_mac);
    }
    m_mac = EVP_MAC_fetch(NULL, "CMAC", NULL);
    if (m_mac == NULL) {
        std::cout << "EVP_MAC_fetch failed, error: "
                  << ERR_GET_REASON(ERR_get_error()) << std::endl;
        return false;
    }

    if (cipher != NULL)
        params[params_n++] =
            OSSL_PARAM_construct_utf8_string("cipher", (char*)cipher, 0);

    params[params_n] = OSSL_PARAM_construct_end();

    if (m_handle != nullptr) {
        EVP_MAC_CTX_free(m_handle);
    }
    m_handle = EVP_MAC_CTX_new(m_mac);
    if (m_handle == NULL) {
        std::cout << "EVP_MAC_CTX_new failed, error: "
                  << ERR_GET_REASON(ERR_get_error()) << std::endl;
        return false;
    }

    if (EVP_MAC_init(m_handle, m_key, m_key_len, params) != 1) {
        std::cout << "EVP_MAC_init failed, error : "
                  << ERR_GET_REASON(ERR_get_error()) << std::endl;
        return false;
    }
    return true;
}

bool
OpenSSLCmacBase::MacUpdate(const alcp_cmac_data_t& data)
{
    if (EVP_MAC_update(m_handle, data.m_msg, data.m_msg_len) != 1) {
        std::cout << "EVP_MAC_update failed, error : "
                  << ERR_GET_REASON(ERR_get_error()) << std::endl;
        return false;
    }
    return true;
}

bool
OpenSSLCmacBase::MacFinalize(const alcp_cmac_data_t& data)
{
    size_t outsize = data.m_cmac_len;

    if (EVP_MAC_final(m_handle, data.m_cmac, &outsize, data.m_cmac_len) != 1) {
        std::cout << "EVP_MAC_final failed, error : "
                  << ERR_GET_REASON(ERR_get_error()) << std::endl;
        return false;
    }
    return true;
}

bool
OpenSSLCmacBase::MacReset()
{
    if (EVP_MAC_init(m_handle, m_key, m_key_len, nullptr) != 1) {
        std::cout << "EVP_MAC_init failed, error : "
                  << ERR_GET_REASON(ERR_get_error()) << std::endl;
        return false;
    }
    return true;
}

} // namespace alcp::testing