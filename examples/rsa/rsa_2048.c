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
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alcp/digest.h"
#include "alcp/rng.h"
#include "alcp/rsa.h"

static const Uint8 Modulus[] = {
    0xae, 0xdd, 0x0e, 0x10, 0xa5, 0xcc, 0xc0, 0x86, 0xfd, 0xdb, 0xef, 0x26,
    0xaa, 0x5b, 0x60, 0xa2, 0x67, 0xc7, 0x0e, 0x50, 0x5c, 0x91, 0x32, 0xc1,
    0x95, 0x27, 0x71, 0xee, 0x30, 0xc6, 0x15, 0x93, 0x77, 0xea, 0x34, 0x8c,
    0x35, 0x67, 0x2e, 0x48, 0xb5, 0x96, 0x77, 0x97, 0x0a, 0x49, 0x74, 0x5d,
    0x44, 0x69, 0x3b, 0xee, 0xb9, 0xa4, 0x1d, 0x75, 0x50, 0xfe, 0x89, 0xa9,
    0xd4, 0xfc, 0x66, 0xbb, 0x4e, 0xca, 0x57, 0xf9, 0xaf, 0x06, 0x35, 0x42,
    0x0c, 0x5b, 0x91, 0x13, 0xf9, 0x1f, 0x7b, 0x16, 0x88, 0xc8, 0x0e, 0x3c,
    0xc2, 0x20, 0x73, 0x39, 0x77, 0xf9, 0x01, 0x58, 0xa2, 0x15, 0x0a, 0x17,
    0x7d, 0x83, 0xb3, 0x5c, 0xcc, 0x23, 0x2d, 0xe4, 0x99, 0xb8, 0x14, 0xf4,
    0x60, 0x61, 0x7a, 0x8e, 0x41, 0x5f, 0x1e, 0x15, 0xe3, 0xe6, 0x46, 0x73,
    0xda, 0xd8, 0xa7, 0xe4, 0xab, 0xda, 0x86, 0xdd, 0x34, 0xdf, 0x9c, 0x28,
    0xd2, 0xcd, 0x3d, 0xb2, 0x40, 0x40, 0x4d, 0xf9, 0x24, 0xf3, 0x4c, 0x65,
    0x1a, 0xb7, 0x41, 0x8e, 0xfe, 0x82, 0xc4, 0x55, 0x74, 0xe2, 0x40, 0xa3,
    0xa5, 0x3e, 0x04, 0x3f, 0x1e, 0x48, 0xf0, 0x55, 0x86, 0x2b, 0x75, 0xd0,
    0xaf, 0x05, 0xcf, 0xe0, 0xa6, 0x93, 0x24, 0x94, 0xad, 0x12, 0xd3, 0x1f,
    0xe1, 0x0f, 0x70, 0x86, 0xa5, 0x87, 0xb1, 0x79, 0x53, 0x5e, 0x07, 0x21,
    0x9d, 0x40, 0x63, 0x5d, 0x8c, 0xd0, 0x21, 0xfd, 0x7f, 0xe2, 0xec, 0xbf,
    0x9e, 0x2e, 0x5f, 0x8b, 0x8c, 0x22, 0x0b, 0x2e, 0xf1, 0xda, 0x6d, 0x35,
    0x7d, 0x76, 0x12, 0x8b, 0x7f, 0xf7, 0xc4, 0x7f, 0x45, 0x3b, 0x8c, 0x29,
    0x3f, 0x7e, 0x53, 0x79, 0xc1, 0x33, 0x8e, 0x77, 0xc2, 0xfa, 0xde, 0xc1,
    0xcf, 0xd1, 0x45, 0x8a, 0x6f, 0x7c, 0xf2, 0x3a, 0x57, 0x40, 0x18, 0x3a,
    0x2e, 0x0a, 0xef, 0x67
};

static const Uint8 P[] = {
    0xb8, 0xc7, 0x80, 0xd1, 0xa9, 0xf2, 0x33, 0x7a, 0x1e, 0xbb, 0x57, 0xcc,
    0x0e, 0x4e, 0x97, 0xfb, 0x92, 0xde, 0xa1, 0x7c, 0xee, 0xf5, 0xaa, 0x63,
    0xd0, 0xa8, 0x24, 0xa6, 0x99, 0x89, 0xb5, 0x7d, 0xf0, 0x82, 0x1c, 0x7e,
    0xad, 0x35, 0xc6, 0x46, 0xb9, 0xa7, 0x8f, 0xa7, 0x37, 0x25, 0x12, 0x4e,
    0xdf, 0xfd, 0x7a, 0x74, 0x21, 0x42, 0x2a, 0x98, 0x4d, 0x4b, 0x86, 0xd8,
    0xca, 0xfb, 0x0e, 0x02, 0xf8, 0x17, 0x59, 0xa5, 0x38, 0x73, 0xba, 0xcb,
    0x57, 0xf5, 0x26, 0xa3, 0x57, 0x27, 0x3f, 0x6f, 0xce, 0xb7, 0x46, 0x32,
    0xc7, 0x00, 0x5b, 0xbb, 0xa9, 0x38, 0x61, 0xa0, 0xc3, 0x28, 0xb2, 0x34,
    0x3b, 0x57, 0xa7, 0x2a, 0xe6, 0xdb, 0x28, 0x7e, 0xbe, 0x0b, 0x78, 0x1a,
    0x8e, 0xec, 0x81, 0x89, 0x18, 0xda, 0x1c, 0xa1, 0xb2, 0x80, 0x26, 0x3c,
    0x83, 0x3c, 0xd4, 0xfc, 0xbc, 0xfb, 0xed, 0x59
};

static const Uint8 Q[] = {
    0xf2, 0x43, 0x24, 0x20, 0xce, 0xbc, 0xb0, 0x3a, 0x9a, 0xf4, 0x08, 0xad,
    0xb2, 0xd2, 0x34, 0x63, 0x37, 0x8a, 0xcb, 0xb9, 0xee, 0xa3, 0x7a, 0x30,
    0x19, 0x88, 0xf3, 0xe1, 0x6b, 0xd1, 0x81, 0xbf, 0xb6, 0xb9, 0x90, 0x88,
    0x9b, 0xcd, 0x82, 0x45, 0xa0, 0x7d, 0x8e, 0x7e, 0xe1, 0x3a, 0xc3, 0x62,
    0x30, 0x90, 0x0d, 0xf2, 0x0b, 0x3c, 0x37, 0x59, 0x28, 0xcd, 0x67, 0x08,
    0xdf, 0x78, 0x13, 0x4b, 0x1d, 0xaa, 0xee, 0x30, 0x00, 0x49, 0x00, 0xe8,
    0x6c, 0x20, 0x6f, 0x96, 0xef, 0x9c, 0x7e, 0x8d, 0x32, 0x11, 0x12, 0x07,
    0xfa, 0x33, 0xf8, 0x1d, 0x1a, 0xb3, 0xe0, 0x0b, 0xc0, 0x71, 0x3c, 0xb5,
    0x72, 0x3c, 0x47, 0x16, 0x04, 0x8b, 0xb4, 0x8c, 0x41, 0xf0, 0x44, 0x24,
    0x29, 0xb7, 0x5a, 0xe3, 0x1b, 0x89, 0xe7, 0x53, 0xa8, 0x33, 0xe0, 0x5e,
    0x14, 0xeb, 0x5b, 0xfc, 0xec, 0x7e, 0x6a, 0xbf
};

static const Uint8 DP[] = {
    0x54, 0x29, 0xf3, 0x00, 0x0c, 0xf3, 0x98, 0x04, 0xe8, 0xd8, 0x96, 0x5e,
    0x08, 0xaa, 0x3d, 0xc9, 0xc6, 0x15, 0x07, 0xe3, 0x5b, 0x08, 0xa4, 0xea,
    0xc0, 0x10, 0xc6, 0x58, 0xe8, 0x18, 0x74, 0x85, 0x7f, 0xb6, 0x13, 0xfa,
    0x93, 0x34, 0xaa, 0x32, 0x6e, 0xbf, 0xe6, 0xcb, 0xd8, 0x6f, 0x57, 0x4e,
    0x7b, 0xf1, 0xfe, 0x03, 0xc5, 0x5e, 0x58, 0xfe, 0x74, 0x3e, 0x91, 0x96,
    0x4f, 0xa6, 0x58, 0xb4, 0x7b, 0x82, 0x4f, 0x3f, 0xd5, 0x5d, 0xc9, 0x58,
    0x73, 0xa0, 0xe3, 0x4f, 0x85, 0x14, 0x08, 0x6e, 0x09, 0xef, 0x2a, 0xd7,
    0x58, 0x13, 0x4e, 0xb5, 0x44, 0x97, 0xbc, 0xc8, 0x37, 0xfc, 0x62, 0x67,
    0x2e, 0x1c, 0x77, 0xb5, 0x2f, 0xdf, 0xe5, 0x2b, 0x0d, 0xaf, 0x35, 0xae,
    0x8b, 0x29, 0x28, 0xbb, 0x64, 0x89, 0x7c, 0x7f, 0x1e, 0x4a, 0x06, 0xa0,
    0x8b, 0x7a, 0x7a, 0xdc, 0xff, 0xcb, 0x94, 0x49
};

static const Uint8 DQ[] = {
    0x56, 0xce, 0x7e, 0x14, 0x8f, 0x5f, 0x87, 0x1a, 0x08, 0xc9, 0xe6, 0x8e,
    0x2e, 0xe4, 0x29, 0x47, 0x5f, 0xf0, 0x88, 0xdd, 0x5f, 0xc8, 0x0e, 0x11,
    0x4c, 0x25, 0x09, 0x96, 0x3d, 0x66, 0xfd, 0xc1, 0xef, 0x3c, 0x80, 0xb0,
    0xa2, 0x7b, 0x39, 0xf1, 0xae, 0xf7, 0x2e, 0x67, 0x02, 0x57, 0x67, 0x09,
    0x38, 0xf3, 0x75, 0x3b, 0xc4, 0x90, 0xd8, 0x18, 0x47, 0x89, 0x8a, 0x20,
    0xe0, 0xca, 0x0a, 0xc7, 0xc0, 0xa2, 0xad, 0xe4, 0x5f, 0x45, 0xc9, 0x60,
    0x7e, 0xd6, 0x04, 0x86, 0x25, 0xe7, 0x82, 0x65, 0x1f, 0x8a, 0x84, 0x56,
    0x7d, 0x6d, 0xbf, 0xba, 0xd6, 0x05, 0x9c, 0x03, 0x39, 0xfa, 0x99, 0x51,
    0x3e, 0xd4, 0xa0, 0x78, 0x20, 0x3a, 0xda, 0xff, 0xe2, 0xe4, 0xaf, 0xd5,
    0xf1, 0x68, 0xb4, 0xd5, 0x69, 0xd9, 0xb9, 0x1c, 0xfd, 0xc9, 0x50, 0xdd,
    0x05, 0x4b, 0xec, 0x53, 0x2d, 0x7e, 0x82, 0xcb
};

static const Uint8 QINV[] = {
    0x29, 0x46, 0xdd, 0xbd, 0x16, 0x47, 0x73, 0xb8, 0x80, 0x88, 0x05, 0xe1,
    0x2b, 0x30, 0xb1, 0x58, 0x25, 0x59, 0xe6, 0x18, 0x54, 0xd6, 0x9e, 0xb8,
    0xc5, 0xb6, 0xe4, 0x07, 0xa1, 0xdd, 0x34, 0x82, 0x61, 0x46, 0xb0, 0x8b,
    0x1d, 0x96, 0xd5, 0x1d, 0x6f, 0x0b, 0x5f, 0xfa, 0xa0, 0xaa, 0x1c, 0xed,
    0x40, 0x9a, 0x5a, 0xf5, 0x08, 0x35, 0xa3, 0x61, 0x22, 0x11, 0x34, 0xd3,
    0xcf, 0x9f, 0xea, 0x7b, 0xb5, 0x41, 0x65, 0x16, 0xfb, 0x58, 0x01, 0x0d,
    0x65, 0x1d, 0x39, 0x16, 0x4e, 0x76, 0xbe, 0x12, 0x32, 0x43, 0x72, 0x13,
    0xd0, 0xe8, 0xdc, 0x9d, 0x5a, 0xdb, 0xaa, 0xe4, 0x77, 0x52, 0x89, 0xcf,
    0xf9, 0xb0, 0x78, 0x59, 0xa9, 0x8c, 0x9e, 0x99, 0x96, 0x0c, 0xfd, 0x9d,
    0x12, 0x56, 0xd0, 0x19, 0x81, 0x10, 0x18, 0xf9, 0x4e, 0x54, 0x92, 0x34,
    0x49, 0x41, 0x2e, 0xd9, 0xc0, 0xe6, 0xd2, 0xc8
};

static const Uint64 PublicKeyExponent = 0x10001;

#define ALCP_PRINT_TEXT(I, L, S)                                               \
    printf("%s\n", S);                                                         \
    for (int x = 0; x < L; x++) {                                              \
        printf(" %02x", *(I + x));                                             \
    }                                                                          \
    printf("\n");

static alc_error_t
create_demo_session(alc_rsa_handle_t* s_rsa_handle)
{
    alc_error_t err;

    Uint64 size           = alcp_rsa_context_size();
    s_rsa_handle->context = malloc(size);

    err = alcp_rsa_request(s_rsa_handle);

    return err;
}

static alc_error_t
Rsa_demo(alc_rsa_handle_t* ps_rsa_handle)
{
    alc_error_t err;
    Uint8*      text     = NULL;
    Uint8*      enc_text = NULL;
    Uint8*      dec_text = NULL;

    Uint64 size = sizeof(Modulus);

    err =
        alcp_rsa_set_publickey(ps_rsa_handle, PublicKeyExponent, Modulus, size);
    if (alcp_is_error(err)) {
        printf("\n setting of publc key failed");
        return err;
    }

    enc_text = malloc(size);
    dec_text = malloc(size);
    text     = malloc(size);

    memset(text, 0x31, size);

    ALCP_PRINT_TEXT(text, size, "text")

    printf("\n");
    // todo call hmac drbg / ctr drbg to generate seed
    // for now the seed is random at buffer allocation

    // Encrypt text

    err = alcp_rsa_publickey_encrypt(ps_rsa_handle, text, size, enc_text);

    if (alcp_is_error(err)) {
        printf("\n publc key encrypt failed");
        goto free_buff;
    }

    ALCP_PRINT_TEXT(enc_text, size, "enc_text")
    printf("\n");

    err = alcp_rsa_set_privatekey(
        ps_rsa_handle, DP, DQ, P, Q, QINV, Modulus, sizeof(P));
    if (alcp_is_error(err)) {
        printf("\n setting of publc key failed");
        goto free_buff;
    }

    err = alcp_rsa_privatekey_decrypt(
        ps_rsa_handle, ALCP_RSA_PADDING_NONE, enc_text, size, dec_text);

    if (alcp_is_error(err)) {
        printf("\n private key decryption failed");
        goto free_buff;
    }

    if (memcmp(dec_text, text, size) == 0) {
        err = ALC_ERROR_NONE;
        ALCP_PRINT_TEXT(dec_text, size, "dec_text")
    } else {
        printf("\n decrypted text not matching the original text");
        err = ALC_ERROR_GENERIC;
    }

free_buff:
    free(dec_text);
    free(enc_text);
    free(text);
    return err;
}

int
main(void)
{
    alc_rsa_handle_t s_rsa_handle;
    alc_error_t      err = create_demo_session(&s_rsa_handle);
    if (alcp_is_error(err)) {
        return -1;
    }

    err = Rsa_demo(&s_rsa_handle);
    if (alcp_is_error(err)) {
        return -1;
    }

    alcp_rsa_finish(&s_rsa_handle);
    free(s_rsa_handle.context);
    return 0;
}
