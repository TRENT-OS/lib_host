/*
 * Copyright (C) 2020, Hensoldt Cyber GmbH
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

static FakeDataport_t fsBuf;
FakeDataport_t* hostStorage_dp = &fsBuf;

static OS_Dataport_t dataport = OS_DATAPORT_ASSIGN(hostStorage_dp);

// Private Functions -----------------------------------------------------------

static bool
allocFile(
    void)
{
    static uint8_t empty[1024];
    size_t wr;
    FILE* fp;

    memset(empty, 0xff, sizeof(empty));

    if ((fp = fopen(HOSTSTORAGE_FILE_NAME, "wb")) == NULL)
    {
        Debug_LOG_ERROR("fopen() failed");
        return false;
    }
    fseek(fp, 0, SEEK_SET);

    wr = 0;
    while (wr < HOSTSTORAGE_SIZE)
    {
        fwrite(empty, sizeof(empty), 1, fp);
        wr += sizeof(empty);
    }

    fclose(fp);

    return true;
}

static bool
checkFile(
    void)
{
    struct stat st;

    if (access(HOSTSTORAGE_FILE_NAME, F_OK) != -1)
    {
        stat(HOSTSTORAGE_FILE_NAME, &st);
        Debug_LOG_TRACE("File '%s' exists with %lli bytes (we expect %zu bytes)",
                        HOSTSTORAGE_FILE_NAME, st.st_size, HOSTSTORAGE_SIZE);
        return (st.st_size == HOSTSTORAGE_SIZE);
    }

    return false;
}

// Public Functions -----------------------------------------------------------

OS_Error_t
HostStorage_write(
    size_t  const offset,
    size_t  const size,
    size_t* const written)
{
    FILE* fp;

    if (size > OS_Dataport_getSize(dataport))
    {
        return OS_ERROR_BUFFER_TOO_SMALL;
    }

    if (!checkFile())
    {
        if (!allocFile())
        {
            return OS_ERROR_GENERIC;
        }
        Debug_LOG_INFO("Allocated file '%s' with %zu bytes",
                       HOSTSTORAGE_FILE_NAME,
                       HOSTSTORAGE_SIZE);
    }

    if ((fp = fopen(HOSTSTORAGE_FILE_NAME, "r+b")) == NULL)
    {
        Debug_LOG_ERROR("fopen() failed on '%s'", HOSTSTORAGE_FILE_NAME);
        return OS_ERROR_GENERIC;

    }

    fseek(fp, offset, SEEK_SET);
    *written = fwrite(OS_Dataport_getBuf(dataport), size, 1, fp);
    *written = *written * size;
    fclose(fp);

    return OS_SUCCESS;
}

OS_Error_t
HostStorage_read(
    size_t  const offset,
    size_t  const size,
    size_t* const read)
{
    FILE* fp;

    if (size > OS_Dataport_getSize(dataport))
    {
        return OS_ERROR_BUFFER_TOO_SMALL;
    }

    if (!checkFile())
    {
        if (!allocFile())
        {
            return OS_ERROR_GENERIC;
        }
        Debug_LOG_INFO("Allocated file '%s' with %zu bytes",
                       HOSTSTORAGE_FILE_NAME,
                       HOSTSTORAGE_SIZE);
    }

    if ((fp = fopen(HOSTSTORAGE_FILE_NAME, "rb")) == NULL)
    {
        Debug_LOG_ERROR("fopen() failed on '%s'", HOSTSTORAGE_FILE_NAME);
        return OS_ERROR_GENERIC;
    }

    fseek(fp, offset, SEEK_SET);
    *read = fread(OS_Dataport_getBuf(dataport), size, 1, fp);
    *read = *read * size;
    fclose(fp);

    return OS_SUCCESS;
}

OS_Error_t
HostStorage_erase(
    size_t  const offset,
    size_t  const size,
    size_t* const erased)
{
    if (size > OS_Dataport_getSize(dataport))
    {
        return OS_ERROR_BUFFER_TOO_SMALL;
    }

    memset(OS_Dataport_getBuf(dataport), 0xff, size);

    return HostStorage_write(offset, size, erased);
}

OS_Error_t
HostStorage_getSize(size_t* const size)
{
    *size = HOSTSTORAGE_SIZE;
    return OS_SUCCESS;
}

OS_Error_t
HostStorage_getState(
    uint32_t* flags)
{
    return OS_ERROR_NOT_SUPPORTED;
}