/*
 * Copyright (C) 2020-2024, HENSOLDT Cyber GmbH
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * For commercial licensing, contact: info.cyber@hensoldt.net
 */

#include "OS_Dataport.h"
#include "OS_Error.h"

#include "lib_debug/Debug.h"
#include "lib_host/HostStorage.h"

#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <inttypes.h>

static FakeDataport_t fsBuf;
FakeDataport_t* hostStorage_port = &fsBuf;

static OS_Dataport_t dataport = OS_DATAPORT_ASSIGN(hostStorage_port);

// Allow these to be overriden by user
static char   hostFileName[HOSTSTORAGE_FILE_NAME_MAX] = HOSTSTORAGE_FILE_NAME;
static size_t hostFileSize = HOSTSTORAGE_SIZE;
static size_t hostBlockSz  = HOSTSTORAGE_BLOCK_SZ;

// Private Functions -----------------------------------------------------------

static bool
allocFile(
    void)
{
    static uint8_t empty[1024];
    off_t left;
    FILE* fp;
    int rc;

    memset(empty, 0xff, sizeof(empty));

    if ((fp = fopen(hostFileName, "wb")) == NULL)
    {
        Debug_LOG_ERROR("fopen() failed");
        return false;
    }
    if ((rc = fseek(fp, 0, SEEK_SET)) != 0)
    {
        Debug_LOG_ERROR("fseek() failed on '%s' with %d", hostFileName, rc);
        fclose(fp);
        return false;
    }

    left = hostFileSize;
    while (left >= sizeof(empty))
    {
        left -= fwrite(empty, 1, sizeof(empty), fp);
    }
    if (left > 0)
    {
        left -= fwrite(empty, 1, left, fp);
    }

    fclose(fp);

    return (left == 0);
}

static bool
checkFile(
    void)
{
    struct stat st;

    if (access(hostFileName, F_OK) != -1)
    {
        stat(hostFileName, &st);
        Debug_LOG_TRACE("File '%s' exists with %" PRIiMAX
                        " bytes (we expect %zu bytes)",
                        hostFileName, st.st_size, hostFileSize);
        return (st.st_size == hostFileSize);
    }

    return false;
}

// Public Functions -----------------------------------------------------------

void
HostStorage_setFileSize(
    const size_t sz)
{
    hostFileSize = sz;
}

void
HostStorage_setBlockSize(
    const size_t blockSz)
{
    hostBlockSz = blockSz;
}

void
HostStorage_setFileName(
    const char* name)
{
    strncpy(hostFileName, name, sizeof(hostFileName) - 1);
    hostFileName[sizeof(hostFileName) - 1] = '\0';
}

OS_Error_t
HostStorage_write(
    off_t   const offset,
    size_t  const size,
    size_t* const written)
{
    *written = 0;
    FILE* fp;
    int rc;

    if (size > OS_Dataport_getSize(dataport))
    {
        return OS_ERROR_BUFFER_TOO_SMALL;
    }
    if ((offset + size) > hostFileSize)
    {
        return OS_ERROR_INVALID_PARAMETER;
    }

    if (!checkFile())
    {
        if (!allocFile())
        {
            return OS_ERROR_GENERIC;
        }
        Debug_LOG_INFO("Allocated file '%s' with %zu bytes", hostFileName,
                       hostFileSize);
    }

    if ((fp = fopen(hostFileName, "r+b")) == NULL)
    {
        Debug_LOG_ERROR("fopen() failed on '%s'", hostFileName);
        return OS_ERROR_GENERIC;

    }
    if ((rc = fseek(fp, offset, SEEK_SET)) != 0)
    {
        Debug_LOG_ERROR("fseek() failed on '%s' with %d", hostFileName, rc);
        fclose(fp);
        return OS_ERROR_GENERIC;
    }

    *written = fwrite(OS_Dataport_getBuf(dataport), 1, size, fp);

    fclose(fp);

    return OS_SUCCESS;
}

OS_Error_t
HostStorage_read(
    off_t   const offset,
    size_t  const size,
    size_t* const read)
{
    *read = 0;
    FILE* fp;
    int rc;

    if (size > OS_Dataport_getSize(dataport))
    {
        return OS_ERROR_BUFFER_TOO_SMALL;
    }
    if ((offset + size) > hostFileSize)
    {
        return OS_ERROR_INVALID_PARAMETER;
    }

    if (!checkFile())
    {
        if (!allocFile())
        {
            return OS_ERROR_GENERIC;
        }
        Debug_LOG_INFO("Allocated file '%s' with %zu bytes", hostFileName,
                       hostFileSize);
    }

    if ((fp = fopen(hostFileName, "rb")) == NULL)
    {
        Debug_LOG_ERROR("fopen() failed on '%s'", hostFileName);
        return OS_ERROR_GENERIC;
    }
    if ((rc = fseek(fp, offset, SEEK_SET)) != 0)
    {
        Debug_LOG_ERROR("fseek() failed on '%s' with %d", hostFileName, rc);
        fclose(fp);
        return OS_ERROR_GENERIC;
    }

    *read = fread(OS_Dataport_getBuf(dataport), 1, size, fp);

    fclose(fp);

    return OS_SUCCESS;
}

OS_Error_t
HostStorage_erase(
    off_t  const offset,
    off_t  const size,
    off_t* const erased)
{
    *erased = 0;
    static uint8_t empty[1024];
    FILE* fp;
    off_t left;
    int rc;

    if ((offset + size) > hostFileSize)
    {
        return OS_ERROR_INVALID_PARAMETER;
    }
    if (!checkFile())
    {
        if (!allocFile())
        {
            return OS_ERROR_GENERIC;
        }
        Debug_LOG_INFO("Allocated file '%s' with %zu bytes", hostFileName,
                       hostFileSize);
    }

    memset(empty, 0xff, sizeof(empty));

    if ((fp = fopen(hostFileName, "r+b")) == NULL)
    {
        Debug_LOG_ERROR("fopen() failed on '%s'", hostFileName);
        return OS_ERROR_GENERIC;
    }
    if ((rc = fseek(fp, offset, SEEK_SET)) != 0)
    {
        Debug_LOG_ERROR("fseek() failed on '%s' with %d", hostFileName, rc);
        fclose(fp);
        return OS_ERROR_GENERIC;
    }

    *erased = 0;
    left    = size;
    while (left >= sizeof(empty))
    {
        *erased += fwrite(empty, 1, sizeof(empty), fp);
        left    -= sizeof(empty);
    }
    if (left > 0)
    {
        *erased += fwrite(empty, 1, left, fp);
    }

    fclose(fp);

    return OS_SUCCESS;
}

OS_Error_t
HostStorage_getSize(
    off_t* const size)
{
    if (NULL == size)
    {
        return OS_ERROR_INVALID_PARAMETER;
    }

    *size = hostFileSize;

    return OS_SUCCESS;
}

OS_Error_t
HostStorage_getBlockSize(
    size_t* blockSize)
{
    if (NULL == blockSize)
    {
        return OS_ERROR_INVALID_PARAMETER;
    }

    *blockSize = HOSTSTORAGE_BLOCK_SZ;

    return OS_SUCCESS;
}

OS_Error_t
HostStorage_getState(
    uint32_t* flags)
{
    return OS_ERROR_NOT_SUPPORTED;
}