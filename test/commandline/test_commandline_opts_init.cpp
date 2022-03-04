/**
 * \file test/commandline/test_commandline_opts_init.cpp
 *
 * \brief Unit tests for commandline_opts_init.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <minunit/minunit.h>
#include <string>
#include <vccrypt/mock_suite.h>
#include <vctool/commandline.h>
#include <vpr/allocator/malloc_allocator.h>

#include "../file/mock_file.h"

using namespace std;

/* start of the test suite. */
TEST_SUITE(commandline_opts_init);

/* If no arguments are provided, return an error. */
TEST(no_arguments)
{
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    char* argv[] = { (char*)exe_name.c_str() };
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create the mock file. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS ==
            file_mock_init(
                &f,
                /* stat. */
                [&](file*, const char*, file_stat_st*) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* open. */
                [&](file*, int*, const char*, int, mode_t) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* close. */
                [&](file*, int) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* read. */
                [&](file*, int, void*, size_t, size_t*) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* write. */
                [&](
                    file*, int, const void*, size_t, size_t*) -> int {
                        return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* lseek. */
                [&](file*, int, off_t, file_lseek_whence, off_t*) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                [&](file*, int) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                }));

    /* create a mock crypto suite. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
        vccrypt_mock_suite_options_init(
            &suite, &alloc_opts));

    /* create a builder options instance. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
            vccert_builder_options_init(
                &builder_opts, &alloc_opts, &suite));

    /* calling commandline_opts_init with no arguments should fail. */
    TEST_EXPECT(
        VCTOOL_ERROR_COMMANDLINE_MISSING_ARGUMENT ==
            commandline_opts_init(&opts, &f, &suite, &builder_opts, 0, argv));

    /* clean up. */
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    dispose((disposable_t*)&alloc_opts);
}

/* If a -? is passed as an argument, invoke the help menu. */
TEST(question_mark_argument)
{
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string question_arg = "-?";
    char* argv[] = { (char*)exe_name.c_str(), (char*)question_arg.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create the mock file. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS ==
            file_mock_init(
                &f,
                /* stat. */
                [&](file*, const char*, file_stat_st*) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* open. */
                [&](file*, int*, const char*, int, mode_t) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* close. */
                [&](file*, int) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* read. */
                [&](file*, int, void*, size_t, size_t*) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* write. */
                [&](
                    file*, int, const void*, size_t, size_t*) -> int {
                        return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* lseek. */
                [&](file*, int, off_t, file_lseek_whence, off_t*) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                [&](file*, int) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                }));

    /* create a mock crypto suite. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
        vccrypt_mock_suite_options_init(
            &suite, &alloc_opts));

    /* create a builder options instance. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
            vccert_builder_options_init(
                &builder_opts, &alloc_opts, &suite));

    /* calling commandline_opts_init should succeed. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS ==
            commandline_opts_init(
                &opts, &f, &suite, &builder_opts, argc, argv));

    /* the help command is set. */
    TEST_ASSERT(NULL != opts.cmd);

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    dispose((disposable_t*)&alloc_opts);
}

/* If a -h is passed as an argument, invoke the help menu. */
TEST(h_argument)
{
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string help_arg = "-h";
    char* argv[] = { (char*)exe_name.c_str(), (char*)help_arg.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create the mock file. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS ==
            file_mock_init(
                &f,
                /* stat. */
                [&](file*, const char*, file_stat_st*) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* open. */
                [&](file*, int*, const char*, int, mode_t) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* close. */
                [&](file*, int) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* read. */
                [&](file*, int, void*, size_t, size_t*) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* write. */
                [&](
                    file*, int, const void*, size_t, size_t*) -> int {
                        return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* lseek. */
                [&](file*, int, off_t, file_lseek_whence, off_t*) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                [&](file*, int) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                }));

    /* create a mock crypto suite. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
        vccrypt_mock_suite_options_init(
            &suite, &alloc_opts));

    /* create a builder options instance. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
            vccert_builder_options_init(
                &builder_opts, &alloc_opts, &suite));

    /* calling commandline_opts_init should succeed. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS ==
            commandline_opts_init(
                &opts, &f, &suite, &builder_opts, argc, argv));

    /* the help command is set. */
    TEST_ASSERT(NULL != opts.cmd);

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    dispose((disposable_t*)&alloc_opts);
}
