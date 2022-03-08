/**
 * \file vctool/main.c
 *
 * \brief Main entry point for the vctool utility.
 *
 * \copyright 2020-2022 Velo Payments.  See License.txt for license terms.
 */

#include <stdio.h>
#include <vccert/builder.h>
#include <vccrypt/suite.h>
#include <vctool/file.h>
#include <vctool/command/help.h>
#include <vctool/commandline.h>
#include <vctool/status_codes.h>
#include <vpr/allocator/malloc_allocator.h>

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Main entry point for vctool.
 *
 * \param argc      The number of arguments.
 * \param argv      Arguments to main.
 *
 * \returns 0 on success and non-zero on failure.
 */
int main(int argc, char* argv[])
{
    int retval, release_retval;
    commandline_opts opts;
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    vccert_builder_options_t builder_opts;
    file file;

    /* register the velo v1 suite. */
    vccrypt_suite_register_velo_v1();

    /* initialize the allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* initialize the vccrypt suite. */
    retval =
        vccrypt_suite_options_init(&suite, &alloc_opts, VCCRYPT_SUITE_VELO_V1);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error initializing crypto suite.\n");
        goto cleanup_allocator;
    }

    /* build options. */
    retval =
        vccert_builder_options_init(&builder_opts, &alloc_opts, &suite);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error initializing certificate builder.\n");
        goto cleanup_crypto_suite;
    }

    /* create OS level file abstraction. */
    retval = file_init(&file);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error creating file abstraction layer.\n");
        goto cleanup_builder_opts;
    }

    /* create an RCPR allocator instance. */
    retval = rcpr_malloc_allocator_create(&alloc);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error creating RCPR allocator.\n");
        goto cleanup_file;
    }

    /* parse command-line options. */
    retval =
        commandline_opts_init(
            &opts, alloc, &file, &suite, &builder_opts, argc, argv);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error parsing command-line options.\n\n");
        help_print(stderr);

        goto cleanup_rcpr_allocator;
    }

    /* attempt to execute the command. */
    retval = command_execute(&opts);

    /* clean up opts. */
    dispose((disposable_t*)&opts);

cleanup_rcpr_allocator:
    release_retval = resource_release(rcpr_allocator_resource_handle(alloc));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_file:
    dispose((disposable_t*)&file);

cleanup_builder_opts:
    dispose((disposable_t*)&builder_opts);

cleanup_crypto_suite:
    dispose((disposable_t*)&suite);

cleanup_allocator:
    dispose((disposable_t*)&alloc_opts);

    return retval;
}
