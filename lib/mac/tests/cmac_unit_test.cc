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
#include "alcp/mac/cmac.hh"
#include "alcp/utils/copy.hh"
#include "gtest/gtest.h"
#include <tuple>

#include "alcp/utils/cpuid.hh"

using namespace alcp::base;
using alcp::mac::Cmac;
using alcp::utils::CpuId;

typedef std::tuple<std::vector<Uint8>, // key
                   std::vector<Uint8>, // plaintext
                   std::vector<Uint8>  // mac
                   >
    param_tuple;

typedef std::map<const std::string, param_tuple> known_answer_map_t;

// clang-format off
known_answer_map_t KAT_CmacDataset{
    { "TESTCASE1_AES_128_BLOCK_1_INCOMPLETE",
      { { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
          0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C 
        },
        {},
        { 
          0xBB,0x1D,0x69,0x29,0xE9,0x59,0x37,0x28,
          0x7F,0xA3,0x7D,0x12,0x9B,0x75,0x67,0x46
        }
      } 
    },
    { "TESTCASE2_AES_128_BLOCK_1_COMPLETE",
      { { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
          0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C 
        },
        { 0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96,
          0xE9, 0x3D, 0x7E, 0x11, 0x73, 0x93, 0x17, 0x2A,
        },
        { 0x07, 0x0A, 0x16, 0xB4, 0x6B, 0x4D,0x41, 0x44, 
          0xF7, 0x9B, 0xDD, 0x9D,0xD0, 0x4A, 0x28, 0x7C 
        }
      } 
    },
    {
        "TESTCASE3_AES_128_BLOCK_2_INCOMPLETE",
      {
        { 0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,
          0x15,0x88,0x09,0xCF,0x4F,0x3C
        },
        {
         0x6B,0xC1,0xBE,0xE2,0x2E,0x40,0x9F,0x96,0xE9,0x3D,0x7E,
         0x11,0x73,0x93,0x17,0x2A,0xAE,0x2D,0x8A,0x57    
        },
        {
          0x7D,0x85,0x44,0x9E,0xA6,0xEA,0x19,0xC8,0x23,0xA7,0xBF,
          0x78,0x83,0x7D,0xFA,0xDE
        }
      }
    },
        {
        "TESTCASE4_AES_128_BLOCK_4_COMPLETE",
      {
        { 0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,
          0x15,0x88,0x09,0xCF,0x4F,0x3C
        },
        {
         0x6B,0xC1,0xBE,0xE2,0x2E,0x40,0x9F,0x96,0xE9,0x3D,0x7E,0x11,0x73,
         0x93,0x17,0x2A,0xAE,0x2D,0x8A,0x57,0x1E,0x03,0xAC,0x9C,0x9E,0xB7,
         0x6F,0xAC,0x45,0xAF,0x8E,0x51,0x30,0xC8,0x1C,0x46,0xA3,0x5C,0xE4,
         0x11,0xE5,0xFB,0xC1,0x19,0x1A,0x0A,0x52,0xEF,0xF6,0x9F,0x24,0x45,
         0xDF,0x4F,0x9B,0x17,0xAD,0x2B,0x41,0x7B,0xE6,0x6C,0x37,0x10
        },
        {
          0x51,0xF0,0xBE,0xBF,0x7E,0x3B,0x9D,0x92,0xFC,0x49,0x74,0x17,0x79,
          0x36,0x3C,0xFE
        }
      }
    }
    ,
    {"TESTCASE5_AES_256_BLOCK_1_COMPLETE",
      {
        { 0x60, 0x3D, 0xEB, 0x10, 0x15, 0xCA, 0x71, 0xBE,
            0x2B, 0x73, 0xAE, 0xF0, 0x85, 0x7D, 0x77, 0x81,
            0x1F, 0x35, 0x2C, 0x07, 0x3B, 0x61, 0x08, 0xD7,
            0x2D, 0x98, 0x10, 0xA3, 0x09, 0x14, 0xDF, 0xF4 
        },
        { 0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40,0x9F, 0x96, 
            0xE9, 0x3D, 0x7E, 0x11,0x73, 0x93, 0x17, 0x2A 
        },
        { 0x28, 0xA7, 0x02, 0x3F, 0x45, 0x2E,0x8F, 0x82, 
            0xBD, 0x4B, 0xF2, 0x8D,0x8C, 0x37, 0xC3, 0x5C 
        }
      },
    },
      {
        "TESTCASE6_AES_192_BLOCK_1_INCOMPLETE",
      {
        { 0x8E, 0x73, 0xB0, 0xF7, 0xDA, 0x0E, 0x64, 0x52,
          0xC8, 0x10, 0xF3, 0x2B, 0x80, 0x90, 0x79, 0xE5,
          0x62, 0xF8, 0xEA, 0xD2, 0x52, 0x2C, 0x6B, 0x7B 
        },
        {
         0x6B,0xC1,0xBE,0xE2,0x2E,0x40,0x9F,0x96,0xE9,0x3D,
         0x7E,0x11,0x73,0x93,0x17,0x2A    
        },
        {
          0x9E,0x99,0xA7,0xBF,0x31,0xE7,0x10,0x90,0x06,0x62,
          0xF6,0x5E,0x61,0x7C,0x51,0x84
        }
      }
      }
      ,
        {
        "TESTCASE7_AES_128_BLOCK_2_COMPLETE",
      {
        { 0x3c,0x1b,0xaf,0x0d,0x91,0x5e,0x5a,0xec,0x92,0xbb,0x62,0xba,0xba,0xd0,0xba,0x2c
        },
        {
         0xf8,0xf2,0x42,0x4c,0x2d,0xc0,0xd0,0xf3,0x82,0x1a,0xf7,0x24,0x40,0x38,0xda,0x08,0x32,0xc5,0x47,0xbe,0x4f,0xf0,0x85,0x0b,0x98,0xc0,0x4d,0x4d,0x44,0xa7,0x16,0xb1   
        },
        {
          0xe1,0x7e,0xa6,0x86,0x21,0x29,0xd6,0xb9
        }
      }
      }

    
    
    
};
// clang-format on

class CMACFuncionalityTest
    : public ::testing::TestWithParam<std::pair<const std::string, param_tuple>>
{
  public:
    std::unique_ptr<Cmac> m_cmac;
    std::vector<Uint8>    m_key, m_plain_text, m_expected_mac, m_mac;

    void SetUp() override
    {
        const auto cParams      = GetParam();
        auto       tuple_values = cParams.second;

        tie(m_key, m_plain_text, m_expected_mac) = tuple_values;
        m_cmac                                   = std::make_unique<Cmac>();
        m_cmac->init(
            &m_key[0],
            static_cast<Uint64>(
                m_key.size())); // m_cmac->init(&m_key[0],
                                // static_cast<Uint64>(m_key.size()), NULL, 0);

        m_mac = std::vector<Uint8>(m_expected_mac.size());
        SetReserve(m_plain_text);
    }

    void splitToEqualHalves(std::vector<Uint8>& singleblock,
                            std::vector<Uint8>& block1,
                            std::vector<Uint8>& block2)
    {
        int block1_size = singleblock.size() / 2;

        block1 = std::vector<Uint8>(singleblock.begin(),
                                    singleblock.begin() + block1_size);
        block2 = std::vector<Uint8>(singleblock.begin() + block1_size,
                                    singleblock.end());

        assert(block1.size() + block2.size() == singleblock.size());
    }

    inline void SetReserve(std::vector<Uint8>& var)
    {
        if (var.size() == 0)
            var.reserve(1);
    }
};

TEST_P(CMACFuncionalityTest, CMAC_SINGLE_UPDATE)
{

    alc_error_t err = ALC_ERROR_NONE;
    err             = m_cmac->update(&m_plain_text[0], m_plain_text.size());
    ASSERT_TRUE(err == ALC_ERROR_NONE);

    err = m_cmac->finalize(&m_mac[0], m_mac.size());
    ASSERT_TRUE(err == ALC_ERROR_NONE);
    EXPECT_EQ(m_mac, m_expected_mac);
}

TEST_P(CMACFuncionalityTest, CMAC_UPDATE_FINALIZE)
{

    alc_error_t err = ALC_ERROR_NONE;
    err             = m_cmac->update(&m_plain_text[0], m_plain_text.size());
    ASSERT_TRUE(err == ALC_ERROR_NONE);
    err = m_cmac->finalize(&m_mac[0], m_mac.size());
    ASSERT_TRUE(err == ALC_ERROR_NONE);
    EXPECT_EQ(m_mac, m_expected_mac);
}

TEST_P(CMACFuncionalityTest, CMAC_MULTIPLE_UPDATE)
{

    std::vector<Uint8> block1, block2;
    splitToEqualHalves(m_plain_text, block1, block2);

    assert(block1.size() <= m_plain_text.size());
    assert(block2.size() <= m_plain_text.size());

    SetReserve(block1);
    SetReserve(block2);
    alc_error_t err = ALC_ERROR_NONE;
    err             = m_cmac->update(&block1[0], block1.size());
    ASSERT_TRUE(err == ALC_ERROR_NONE);
    err = m_cmac->update(&block2[0], block2.size());
    ASSERT_TRUE(err == ALC_ERROR_NONE);
    err = m_cmac->finalize(&m_mac[0], m_mac.size());
    ASSERT_TRUE(err == ALC_ERROR_NONE);
    EXPECT_EQ(m_mac, m_expected_mac);
}

TEST_P(CMACFuncionalityTest, CMAC_RESET)
{
    m_cmac->update(&m_plain_text[0], m_plain_text.size());
    m_cmac->reset();
    m_cmac->update(&m_plain_text[0], m_plain_text.size());
    m_cmac->finalize(&m_mac[0], m_mac.size());

    EXPECT_EQ(m_mac, m_expected_mac);
}

TEST(CMACRobustnessTest, CMAC_CreateObject)
{
    Cmac cmac2;
}

TEST(CMACRobustnessTest, CMAC_callUpdateOnNullKey)
{
    Cmac  cmac2;
    Uint8 data[20]{};

    alc_error_t err = cmac2.update(data, sizeof(data));
    ASSERT_EQ(err, ALC_ERROR_BAD_STATE);
}

TEST(CMACRobustnessTest, CMAC_callFinalizeOnNullKey)
{
    Cmac  cmac2;
    Uint8 data[20]{};

    alc_error_t err = cmac2.finalize(data, sizeof(data));
    ASSERT_EQ(err, ALC_ERROR_BAD_STATE);
}

TEST(CMACRobustnessTest, CMAC_callUpdateAfterFinalize)
{
    Uint8 key[16]{};
    Cmac  cmac2;

    alc_error_t err = ALC_ERROR_NONE;
    cmac2.init(key, sizeof(key));
    ASSERT_TRUE(err == ALC_ERROR_NONE);

    err = cmac2.finalize(nullptr, 0);
    ASSERT_TRUE(err == ALC_ERROR_NONE);
    err = cmac2.update(nullptr, 0);
    ASSERT_EQ(err, ALC_ERROR_BAD_STATE);
}

TEST(CMACRobustnessTest, CMAC_callFinalizeTwice)
{
    Uint8 key[16]{};
    Cmac  cmac2;

    /* FIXME: Usage of Status is not done properly in the library. Status and
     alc_error_t usage is mixed. */
    // step 1: remove Status usage fully. ==> PENDING;
    // step 2: Add Status for all api-s one shot ==> PENDING.
    alc_error_t err = ALC_ERROR_NONE;
    err             = cmac2.init(key, sizeof(key));
    EXPECT_EQ(err, ALC_ERROR_NONE);

    err = cmac2.finalize(nullptr, 0);
    ASSERT_TRUE(err == ALC_ERROR_NONE);

    err = cmac2.finalize(nullptr, 0);
    ASSERT_EQ(err, ALC_ERROR_BAD_STATE);
}

TEST(CMACRobustnessTest, CMAC_wrongKeySize)
{
    GTEST_SKIP() << "Skipping the test due to failure in Rijindael";
    Uint8 key[30]{};

    Cmac cmac2;

    // FIXME: Rijindael init should be returning proper error status and this
    // should not be passing
    alc_error_t err = ALC_ERROR_NONE;
    err             = cmac2.init(key, sizeof(key));
    EXPECT_EQ(err, ALC_ERROR_NONE);
}

INSTANTIATE_TEST_SUITE_P(
    CMACTest,
    CMACFuncionalityTest,
    testing::ValuesIn(KAT_CmacDataset),
    [](const testing::TestParamInfo<CMACFuncionalityTest::ParamType>& info) {
        return info.param.first;
    });