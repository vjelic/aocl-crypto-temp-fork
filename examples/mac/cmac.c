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

#include "alcp/alcp.h"
#include <inttypes.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
void
printHashAsHexString(Uint8* hash, int length)
{
    size_t i;
    for (i = 0; i < length; i++) {
        printf("%02x", hash[i]);
    }
}

int
compareArray(Uint8* a1, Uint64 a1_len, Uint8* a2, Uint64 a2_len)
{
    if (a1_len != a2_len) {
        return 1;
    }
    return memcmp(a1, a2, a1_len);
}

static alc_mac_handle_t handle;

alc_error_t
run_cmac(Uint8*         cipherText,
         Uint32         cipherTextLen,
         Uint8*         mac,
         Uint32         mac_size,
         const Uint8*   key,
         Uint32         key_size,
         alc_mac_info_t macinfo)
{

    alc_error_t err = ALC_ERROR_NONE;

    handle.ch_context = malloc(alcp_mac_context_size());

    if (handle.ch_context == NULL) {
        return ALC_ERROR_GENERIC;
    }

    err = alcp_mac_request(&handle, ALC_MAC_CMAC);
    if (alcp_is_error(err)) {
        printf("Error Occurred on MAC Request - %10" PRId64 "\n", err);
        return err;
    }

    err = alcp_mac_init(&handle, key, key_size, &macinfo);
    if (alcp_is_error(err)) {
        printf("Error Occurred on MAC init - %10" PRId64 "\n", err);
        return err;
    }

    // Update can be called multiple times with smaller chunks of the
    // cipherText
    err = alcp_mac_update(&handle, cipherText, cipherTextLen);
    if (alcp_is_error(err)) {
        printf("Error Occurred on MAC Update\n");
        return err;
    }

    err = alcp_mac_finalize(&handle, mac, mac_size);
    if (alcp_is_error(err)) {
        printf("Error Occurred on MAC Finalize\n");
        return err;
    }

    alcp_mac_finish(&handle);
    free(handle.ch_context);
    return err;
}

bool
validateMAC(char*  hmac_string,
            Uint8* key,
            Uint32 keylen,
            Uint8* cipherText,
            Uint32 cipherTextLen,
            Uint8* mac,
            Uint32 macLen,
            Uint8* expectedMac,
            Uint32 expectedMacLength)
{

    bool isvalidated = false;
    printf("%s", hmac_string);
    printf(" : ");
    printf("\n\t");
    printf("KEY = \t\t");
    printHashAsHexString(key, keylen);
    printf("\n\t");
    printf("CipherText = \t");
    printHashAsHexString(cipherText, cipherTextLen);
    printf("\n\t");
    printf("MAC = \t\t");
    printHashAsHexString(mac, macLen);
    printf("\n\t");
    printf("Expected MAC = \t");
    printHashAsHexString(expectedMac, expectedMacLength);
    printf("\n\t");
    if (!compareArray(mac, macLen, expectedMac, expectedMacLength)) {
        printf("MAC IS VERIFIED");
        isvalidated = true;
    } else {
        printf("INVALID MAC");
    }
    printf("\n");
    printf("=======================");
    printf("\n");

    return isvalidated;
}

int
demo_cmac()
{
    alc_error_t err;

    Uint8 key[] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
                    0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };

    Uint8 cipherText[] = {
        0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96,
        0xE9, 0x3D, 0x7E, 0x11, 0x73, 0x93, 0x17, 0x2A,
    };

    Uint8 expectedMac[] = { 0x07, 0x0A, 0x16, 0xB4, 0x6B, 0x4D, 0x41, 0x44,
                            0xF7, 0x9B, 0xDD, 0x9D, 0xD0, 0x4A, 0x28, 0x7C };

    alc_mac_info_t macinfo = { .cmac.ci_mode = ALC_AES_MODE_NONE };

    Uint64 mac_size = 16;
    Uint8  mac[mac_size];
    err = run_cmac(cipherText,
                   sizeof(cipherText),
                   mac,
                   mac_size,
                   key,
                   sizeof(key),
                   macinfo);
    if (alcp_is_error(err)) {
        printf("Error in CMAC\n");
        return -1;
    } else {

        bool isvalidated = validateMAC("CMAC",
                                       key,
                                       sizeof(key),
                                       cipherText,
                                       sizeof(cipherText),
                                       mac,
                                       sizeof(mac),
                                       expectedMac,
                                       sizeof(expectedMac));
        if (!isvalidated) {
            return -1;
        }
    }
    return 0;
}

int
main(int argc, char const* argv[])
{
    if (demo_cmac() != 0)
        goto out;

    return 0;
out:
    return -1;
}
