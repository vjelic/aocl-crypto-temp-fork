 # Copyright (C) 2022-2024, Advanced Micro Devices. All rights reserved.
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
 

FILE(GLOB AVX2_SRCS "*.cc")

alcp_get_arch_cflags_avx2()

#CMAKE_POLICY(SET CMP0079 NEW)

MESSAGE(STATUS "COMPILE_FLAGS:AVX2:${ARCH_COMPILE_FLAGS}")
ADD_COMPILE_OPTIONS(${ARCH_COMPILE_FLAGS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/include)

IF(ALCP_ENABLE_OFB_MULTI_UPDATE)
	SET_SOURCE_FILES_PROPERTIES(aesni_ofb.cc PROPERTIES COMPILE_FLAGS "-DOFB_MULTI_UPDATE")
ENDIF(ALCP_ENABLE_OFB_MULTI_UPDATE)


ADD_LIBRARY(arch_avx2
	OBJECT ${AVX2_SRCS}
)

IF(ALCP_ENABLE_CIPHER_MULTI_UPDATE)
	TARGET_COMPILE_DEFINITIONS(arch_avx2 PRIVATE "AES_MULTI_UPDATE")	
ENDIF(ALCP_ENABLE_CIPHER_MULTI_UPDATE)

TARGET_LINK_LIBRARIES(alcp PRIVATE arch_avx2)
TARGET_LINK_LIBRARIES(alcp_static PRIVATE arch_avx2)
