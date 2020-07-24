/*
 * Copyright (C) 2020, Hensoldt Cyber GmbH
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
#   define HOSTSTORAGE_SIZE ((size_t)(1 * 1024 * 1024))
#endif

// Equivalent of CAmkES if_OS_Storage interface --------------------------------

OS_Error_t
HostStorage_write(
    size_t  const offset,
    size_t  const size,
    size_t* const written);

OS_Error_t
HostStorage_read(
    size_t  const offset,
    size_t  const size,
    size_t* const read);

OS_Error_t
HostStorage_erase(
    size_t  const offset,
    size_t  const size,
    size_t* const erased);

OS_Error_t
HostStorage_getSize(
    size_t* const size);

OS_Error_t
HostStorage_getState(
    uint32_t* flags);

// Additonal functionality -----------------------------------------------------

void
HostStorage_setFileSize(
    const size_t sz);

void
HostStorage_setFileName(
    const char* name);