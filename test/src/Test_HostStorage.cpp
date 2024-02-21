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
#include "interfaces/if_OS_Storage.h"
#include "lib_host/HostStorage.h"
#include <stdint.h>
#include <limits.h>
}

extern "C"
{
    extern FakeDataport_t* hostStorage_port;
    static if_OS_Storage_t storage =
        IF_OS_STORAGE_ASSIGN(
            HostStorage,
            hostStorage_port);
}

class Test_HostStorage : public testing::Test
{
    protected:
};

TEST(Test_HostStorage, getSize_pos)
{
    off_t left;

    // Get size and make sure it is the original storage size
    ASSERT_EQ(OS_SUCCESS, storage.getSize(&left));
    ASSERT_EQ(HOSTSTORAGE_SIZE, left);
}

TEST(Test_HostStorage, getSize_neg)
{
    ASSERT_EQ(OS_ERROR_INVALID_PARAMETER, storage.getSize(NULL));
}

TEST(Test_HostStorage, getState)
{
    // Doesn't matter if we pass a buffer or not, because HostStorage does not
    // support this call
    ASSERT_EQ(OS_ERROR_NOT_SUPPORTED, storage.getState(NULL));
}

TEST(Test_HostStorage, read_write_erase_read)
{
    uint8_t buf[] = {0, 1, 2, 3, 4, 5, 6, 7};
    size_t wr, rd, sz;
    off_t off, left, er;

    // Make storage size smaller so the best below takes less time
    HostStorage_setFileSize(16 * 4096);

    memcpy(OS_Dataport_getBuf(storage.dataport), buf, sizeof(buf));

    // Write pattern to entire "disk"
    off = 0;
    ASSERT_EQ(OS_SUCCESS, storage.getSize(&left));
    while (left > 0)
    {
        ASSERT_EQ(OS_SUCCESS, storage.write(off, sizeof(buf), &wr));
        ASSERT_EQ(wr, sizeof(buf));
        off  += wr;
        left -= wr;
    }

    // Read pattern back
    off = 0;
    ASSERT_EQ(OS_SUCCESS, storage.getSize(&left));
    while (left > 0)
    {
        ASSERT_EQ(OS_SUCCESS, storage.read(off, sizeof(buf), &rd));
        ASSERT_EQ(rd, sizeof(buf));
        ASSERT_EQ(0, memcmp(OS_Dataport_getBuf(storage.dataport), buf, sizeof(buf)));
        off  += rd;
        left -= rd;
    }

    // Erase everything, but in little steps..
    off = 0;
    ASSERT_EQ(OS_SUCCESS, storage.getSize(&left));
    while (left > 0)
    {
        ASSERT_EQ(OS_SUCCESS, storage.erase(off, sizeof(buf), &er));
        ASSERT_EQ(er, sizeof(buf));
        off  += er;
        left -= er;
    }

    // Read bigger chunks (max size allowed by port) and check if 0xff, as we
    // want to pretend we are a FLASH (where empty = 0xff).
    off = 0;
    sz  = OS_Dataport_getSize(storage.dataport);
    ASSERT_EQ(OS_SUCCESS, storage.getSize(&left));
    while (left > 0)
    {
        ASSERT_EQ(OS_SUCCESS, storage.read(off, sz, &rd));
        ASSERT_EQ(rd, sz);
        for (size_t i = 0; i < sz; i++)
        {
            ASSERT_EQ(0xff, *((uint8_t*)OS_Dataport_getBuf(storage.dataport) + i));
        }
        off  += rd;
        left -= rd;
    }

    // Reset storage file size for other tests
    HostStorage_setFileSize(HOSTSTORAGE_SIZE);
}

TEST(Test_HostStorage, read_pos)
{
    off_t sz;
    size_t rd;

    // Read exactly at end of storage size, and then get 1 byte read
    ASSERT_EQ(OS_SUCCESS, storage.getSize(&sz));
    ASSERT_EQ(OS_SUCCESS, storage.read(sz - 1, 1, &rd));
    ASSERT_EQ(1, rd);
}

TEST(Test_HostStorage, read_neg)
{
    off_t sz;
    size_t rd;

    // Too big for port buffer
    sz  = OS_Dataport_getSize(storage.dataport);
    ASSERT_EQ(OS_ERROR_BUFFER_TOO_SMALL, storage.read(0, sz + 1, &rd));

    // Read beyond size of storage
    ASSERT_EQ(OS_SUCCESS, storage.getSize(&sz));
    ASSERT_EQ(OS_ERROR_INVALID_PARAMETER, storage.read(sz - 15, 16, &rd));
}

TEST(Test_HostStorage, write_pos)
{
    off_t sz;
    size_t wr;

    // Write exactly at end of storage size, and then get 1 byte written
    ASSERT_EQ(OS_SUCCESS, storage.getSize(&sz));
    ASSERT_EQ(OS_SUCCESS, storage.write(sz - 1, 1, &wr));
    ASSERT_EQ(1, wr);
}

TEST(Test_HostStorage, write_neg)
{
    off_t sz;
    size_t wr;

    // Too big for port buffer
    sz  = OS_Dataport_getSize(storage.dataport);
    ASSERT_EQ(OS_ERROR_BUFFER_TOO_SMALL, storage.write(0, sz + 1, &wr));

    // Write beyond size of storage
    ASSERT_EQ(OS_SUCCESS, storage.getSize(&sz));
    ASSERT_EQ(OS_ERROR_INVALID_PARAMETER, storage.write(sz - 15, 16, &wr));
}

TEST(Test_HostStorage, erase_pos)
{
    off_t sz, er;

    // Erase the whole thing at once
    ASSERT_EQ(OS_SUCCESS, storage.getSize(&sz));
    ASSERT_EQ(OS_SUCCESS, storage.erase(0, sz, &er));
    ASSERT_EQ(sz, er);

    // Erase exactly at end of storage size, and then get 1 byte written
    ASSERT_EQ(OS_SUCCESS, storage.getSize(&sz));
    ASSERT_EQ(OS_SUCCESS, storage.erase(sz - 1, 1, &er));
    ASSERT_EQ(1, er);
}

TEST(Test_HostStorage, erase_neg)
{
    off_t sz, er;

    // Erase beyond size of storage
    ASSERT_EQ(OS_SUCCESS, storage.getSize(&sz));
    ASSERT_EQ(OS_ERROR_INVALID_PARAMETER, storage.erase(sz - 15, 16, &er));
}

TEST(Test_HostStorage, change_file)
{
    uint8_t buf[] = {0, 1, 2, 3, 4, 5, 6, 7};
    size_t sz, wr, rd;

    sz = OS_Dataport_getSize(storage.dataport);

    // Write something and read it back
    memcpy(OS_Dataport_getBuf(storage.dataport), buf, sizeof(buf));
    ASSERT_EQ(OS_SUCCESS, storage.write(sz, sizeof(buf), &wr));
    ASSERT_EQ(wr, sizeof(buf));
    ASSERT_EQ(OS_SUCCESS, storage.read(sz, sizeof(buf), &rd));
    ASSERT_EQ(rd, sizeof(buf));
    ASSERT_EQ(0, memcmp(OS_Dataport_getBuf(storage.dataport), buf, sizeof(buf)));

    // This should create a new file which is exactly as big as the dataport
    HostStorage_setFileSize(sz);

    // Check new file is actually "erased"
    ASSERT_EQ(OS_SUCCESS, storage.read(0, sz, &rd));
    ASSERT_EQ(rd, sz);
    for (size_t i = 0; i < rd; i++)
    {
        ASSERT_EQ(0xff, *((uint8_t*)OS_Dataport_getBuf(storage.dataport) + i));
    }
    // And we should not be able to read beyond the new size
    ASSERT_EQ(OS_ERROR_INVALID_PARAMETER, storage.read(sz, 1, &rd));

    // Write something again
    memcpy(OS_Dataport_getBuf(storage.dataport), buf, sizeof(buf));
    ASSERT_EQ(OS_SUCCESS, storage.write(0, sizeof(buf), &wr));
    ASSERT_EQ(wr, sizeof(buf));

    // This should create a new file
    HostStorage_setFileName("some-file-name-not-equal-to-nvm_06");

    // Check new file is actually "erased"
    ASSERT_EQ(OS_SUCCESS, storage.read(0, sz, &rd));
    ASSERT_EQ(rd, sz);
    for (size_t i = 0; i < rd; i++)
    {
        ASSERT_EQ(0xff, *((uint8_t*)OS_Dataport_getBuf(storage.dataport) + i));
    }
}
