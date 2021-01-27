/*
 * Copyright (C) 2020, HENSOLDT Cyber GmbH
 */

#pragma once

#include "OS_Error.h"

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

#define HOSTSTORAGE_FILE_NAME_MAX (64)

#if !defined(HOSTSTORAGE_FILE_NAME)
#   define HOSTSTORAGE_FILE_NAME "nvm_06"
#endif

#if !defined(HOSTSTORAGE_SIZE)
#   define HOSTSTORAGE_SIZE (((off_t)1) * 1024 * 1024)
#endif

#if !defined(HOSTSTORAGE_BLOCK_SZ)
#   define HOSTSTORAGE_BLOCK_SZ     1U
#endif

// Equivalent of CAmkES if_OS_Storage interface --------------------------------

OS_Error_t
HostStorage_write(
    off_t   const offset,
    size_t  const size,
    size_t* const written);

OS_Error_t
HostStorage_read(
    off_t   const offset,
    size_t  const size,
    size_t* const read);

OS_Error_t
HostStorage_erase(
    off_t  const offset,
    off_t  const size,
    off_t* const erased);

OS_Error_t
HostStorage_getSize(
    off_t*  const size);

OS_Error_t
HostStorage_getBlockSize(
    size_t* blockSize);

OS_Error_t
HostStorage_getState(
    uint32_t* flags);

// Additonal functionality -----------------------------------------------------

void
HostStorage_setFileSize(
    const size_t sz);

void
HostStorage_setBlockSize(
    const size_t blockSz);

void
HostStorage_setFileName(
    const char* name);