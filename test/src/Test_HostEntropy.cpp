/*
 * Copyright (C) 2019-2024, HENSOLDT Cyber GmbH
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * For commercial licensing, contact: info.cyber@hensoldt.net
 */

#include <gtest/gtest.h>

extern "C"
{
#include "OS_Dataport.h"
#include "interfaces/if_OS_Entropy.h"
#include "lib_host/HostEntropy.h"
#include <stdint.h>
#include <limits.h>
}

extern "C"
{
    extern FakeDataport_t* hostEntropy_port;
    static if_OS_Entropy_t entropy =
        IF_OS_ENTROPY_ASSIGN(
            HostEntropy,
            hostEntropy_port);
}

class Test_HostEntropy : public testing::Test
{
    protected:
};

TEST(Test_HostEntropy, read)
{
    size_t sz, zero;

    sz  = OS_Dataport_getSize(entropy.dataport);
    memset(OS_Dataport_getBuf(entropy.dataport), 0, sz);

    // Read as much as dataport allows
    ASSERT_EQ(sz, entropy.read(sz));

    // Make a little check on the amount of zeros we find in the "random" string.
    // We would expect sz/256 occurences (assuming a perfectly random distribution),
    // but the reality is different. Since we do not want any test failuires, lets
    // make sure at least we don't have too many zeroes..
    zero = 0;
    for (size_t i = 0; i < sz; i++)
    {
        zero += (0x00 == (*((uint8_t*)OS_Dataport_getBuf(entropy.dataport) + i))) ? 1 :
                0;
    }
    ASSERT_LE(zero, sz >> 4);
}
