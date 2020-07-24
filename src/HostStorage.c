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

// Allow these to be overriden by user
static char   hostFileName[HOSTSTORAGE_FILE_NAME_MAX] = HOSTSTORAGE_FILE_NAME;
static size_t hostFileSize = HOSTSTORAGE_SIZE;

// Private Functions -----------------------------------------------------------

static bool
allocFile(
    void)
{
    static uint8_t empty[1024];
    size_t wr;
    FILE* fp;

    memset(empty, 0xff, sizeof(empty));

    if ((fp = fopen(hostFileName, "wb")) == NULL)
    {
        Debug_LOG_ERROR("fopen() failed");
        return false;
    }
    fseek(fp, 0, SEEK_SET);

    wr = 0;
    while (wr < hostFileSize)
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

    if (access(hostFileName, F_OK) != -1)
    {
        stat(hostFileName, &st);
        Debug_LOG_TRACE("File '%s' exists with %lli bytes (we expect %zu bytes)",
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
HostStorage_setFileName(
    const char* name)
{
    strncpy(hostFileName, name, sizeof(hostFileName));
}

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
        Debug_LOG_INFO("Allocated file '%s' with %zu bytes", hostFileName,
                       hostFileSize);
    }

    if ((fp = fopen(hostFileName, "r+b")) == NULL)
    {
        Debug_LOG_ERROR("fopen() failed on '%s'", hostFileName);
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
        Debug_LOG_INFO("Allocated file '%s' with %zu bytes", hostFileName,
                       hostFileSize);
    }

    if ((fp = fopen(hostFileName, "rb")) == NULL)
    {
        Debug_LOG_ERROR("fopen() failed on '%s'", hostFileName);
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
HostStorage_getSize(
    size_t* const size)
{
    *size = hostFileSize;
    return OS_SUCCESS;
}

OS_Error_t
HostStorage_getState(
    uint32_t* flags)
{
    return OS_ERROR_NOT_SUPPORTED;
}