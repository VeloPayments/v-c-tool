/**
 * \file command/endorse/certfile_create.c
 *
 * \brief Create a certificate file resource.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <stdlib.h>
#include <string.h>

#include "certfile.h"

RCPR_IMPORT_allocator;
RCPR_IMPORT_resource;

/* forward decls. */
static status certfile_resource_release(resource* r);

/**
 * \brief Create a certificate file resource.
 *
 * \param cert      Receives the pointer to the certificate file created.
 * \param alloc     The allocator to use to create this certificate file.
 * \param filename  The name of the certificate file.
 * \param size      The size of the certificate file.
 */
status certfile_create(
    certfile** cert, RCPR_SYM(allocator)* alloc, char* filename, size_t size)
{
    status retval, release_retval;
    certfile* tmp;

    /* allocate memory for the certificate file. */
    retval = allocator_allocate(alloc, (void**)&tmp, sizeof(certfile));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* initialize data structure. */
    memset(tmp, 0, sizeof(certfile));
    resource_init(&tmp->hdr, &certfile_resource_release);
    tmp->alloc = alloc;
    tmp->size = size;

    /* duplicate the filename. */
    tmp->filename = strdup(filename);
    if (NULL == tmp->filename)
    {
        retval = ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_certfile;
    }

    /* success.  Assign cert to tmp. Caller assumes ownership of cert. */
    *cert = tmp;
    retval = STATUS_SUCCESS;
    goto done;

cleanup_certfile:
    memset(tmp, 0, sizeof(certfile));
    release_retval = allocator_reclaim(alloc, tmp);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}

/**
 * \brief Release a certfile resource.
 *
 * \param r         The resource to release.
 */
static status certfile_resource_release(resource* r)
{
    certfile* cert = (certfile*)r;

    /* cache allocator. */
    allocator* alloc = cert->alloc;

    /* release filename. */
    free(cert->filename);

    /* clear structure. */
    memset(cert, 0, sizeof(*cert));

    /* reclaim memory. */
    return allocator_reclaim(alloc, cert);
}
