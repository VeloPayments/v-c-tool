/**
 * \file command/endorse/certfile.h
 *
 * \brief Declarations for the certificate file interface.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#pragma once

#include <rcpr/allocator.h>
#include <rcpr/resource/protected.h>
#include <rcpr/status.h>

/**
 * \brief Certificate file resource.
 */
typedef struct certfile
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    char* filename;
    size_t size;
} certfile;

/**
 * \brief Create a certificate file resource.
 *
 * \param cert      Receives the pointer to the certificate file created.
 * \param alloc     The allocator to use to create this certificate file.
 * \param filename  The name of the certificate file.
 * \param size      The size of the certificate file.
 */
status certfile_create(
    certfile** cert, RCPR_SYM(allocator)* alloc, const char* filename,
    size_t size);
