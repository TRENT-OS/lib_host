/*
 * Copyright (C) 2020, Hensoldt Cyber GmbH
 */

#include "OS_Dataport.h"
#include "OS_Error.h"

#include "lib_debug/Debug.h"
#include "lib_host/HostEntropy.h"

#include <stdio.h>
#include <stdbool.h>

static FakeDataport_t entropyBuf;
FakeDataport_t* hostEntropy_dp = &entropyBuf;

static OS_Dataport_t dataport = OS_DATAPORT_ASSIGN(hostEntropy_dp);

// Public Functions -----------------------------------------------------------

size_t
HostEntropy_read(
    const size_t len)
{
    FILE* fp;
    size_t sz;

    // Make sure we don't exceed the buffer size
    sz = OS_Dataport_getSize(dataport);
    sz = len > sz ? sz : len;

    // Since the KPT is built on the host, we *should* have /dev/urandom
    // available..
    if ((fp = fopen("/dev/urandom", "r")) != NULL)
    {
        sz = fread(OS_Dataport_getBuf(dataport), 1, sz, fp);
        fclose(fp);
    }
    else
    {
        Debug_LOG_ERROR("Failed to open /dev/urandom");
        sz = 0;
    }

    return sz;
}