# Copyright (C) 2024, Advanced Micro Devices. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software
# without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

INCLUDE(${CMAKE_SOURCE_DIR}/cmake/AlcpFuzzTests.cmake)

# Fuzz targets
ADD_FUZZ_TARGET(test_fuzz_aes_encrypt_cbc.cc)
ADD_FUZZ_TARGET(test_fuzz_aes_decrypt_cbc.cc)

ADD_FUZZ_TARGET(test_fuzz_aes_encrypt_cfb.cc)
ADD_FUZZ_TARGET(test_fuzz_aes_decrypt_cfb.cc)

ADD_FUZZ_TARGET(test_fuzz_aes_encrypt_ctr.cc)
ADD_FUZZ_TARGET(test_fuzz_aes_decrypt_ctr.cc)

ADD_FUZZ_TARGET(test_fuzz_aes_encrypt_ofb.cc)
ADD_FUZZ_TARGET(test_fuzz_aes_decrypt_ofb.cc)

ADD_FUZZ_TARGET(test_fuzz_aes_encrypt_gcm.cc)
ADD_FUZZ_TARGET(test_fuzz_aes_decrypt_gcm.cc)

ADD_FUZZ_TARGET(test_fuzz_aes_encrypt_ccm.cc)
ADD_FUZZ_TARGET(test_fuzz_aes_decrypt_ccm.cc)

ADD_FUZZ_TARGET(test_fuzz_aes_encrypt_siv.cc)
ADD_FUZZ_TARGET(test_fuzz_aes_decrypt_siv.cc)

ADD_FUZZ_TARGET(test_fuzz_aes_encrypt_xts.cc)
ADD_FUZZ_TARGET(test_fuzz_aes_decrypt_xts.cc)

ADD_FUZZ_TARGET(test_fuzz_encrypt_chacha20.cc)
ADD_FUZZ_TARGET(test_fuzz_decrypt_chacha20.cc)

ADD_FUZZ_TARGET(test_fuzz_encrypt_chacha20-poly1305.cc)
ADD_FUZZ_TARGET(test_fuzz_decrypt_chacha20-poly1305.cc)

# Lifecycle test targets
ADD_FUZZ_TARGET(test_fuzz_aes_encrypt_cbc_lifecycle.cc)
ADD_FUZZ_TARGET(test_fuzz_aes_decrypt_cbc_lifecycle.cc)

ADD_FUZZ_TARGET(test_fuzz_aes_encrypt_ofb_lifecycle.cc)
ADD_FUZZ_TARGET(test_fuzz_aes_decrypt_ofb_lifecycle.cc)

ADD_FUZZ_TARGET(test_fuzz_aes_encrypt_ctr_lifecycle.cc)
ADD_FUZZ_TARGET(test_fuzz_aes_decrypt_ctr_lifecycle.cc)

ADD_FUZZ_TARGET(test_fuzz_aes_encrypt_cfb_lifecycle.cc)
ADD_FUZZ_TARGET(test_fuzz_aes_decrypt_cfb_lifecycle.cc)

ADD_FUZZ_TARGET(test_fuzz_aes_encrypt_gcm_lifecycle.cc)
ADD_FUZZ_TARGET(test_fuzz_aes_decrypt_gcm_lifecycle.cc)

ADD_FUZZ_TARGET(test_fuzz_aes_encrypt_ccm_lifecycle.cc)
ADD_FUZZ_TARGET(test_fuzz_aes_decrypt_ccm_lifecycle.cc)

ADD_FUZZ_TARGET(test_fuzz_aes_encrypt_siv_lifecycle.cc)
ADD_FUZZ_TARGET(test_fuzz_aes_decrypt_siv_lifecycle.cc)

ADD_FUZZ_TARGET(test_fuzz_encrypt_chacha20_lifecycle.cc)
ADD_FUZZ_TARGET(test_fuzz_decrypt_chacha20_lifecycle.cc)

ADD_FUZZ_TARGET(test_fuzz_encrypt_chacha20-poly1305_lifecycle.cc)
ADD_FUZZ_TARGET(test_fuzz_decrypt_chacha20-poly1305_lifecycle.cc)


