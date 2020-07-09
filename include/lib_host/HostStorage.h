/*
 * Copyright (C) 2020, Hensoldt Cyber GmbH
 */

#pragma once

#include "OS_Error.h"

#include <stdint.h>
#include <stddef.h>

#if !defined(HOSTSTORAGE_FILE_NAME)
#   define HOSTSTORAGE_FILE_NAME "nvm_06"
#endif

#if !defined(HOSTSTORAGE_SIZE)
#   define HOSTSTORAGE_SIZE ((size_t)(1 * 1024 * 1024))
#endif

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