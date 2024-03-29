/**
 * \file test/commandline/test_commandline_opts_init.cpp
 *
 * \brief Unit tests for commandline_opts_init.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <minunit/minunit.h>
#include <string>
#include <string.h>
#include <vccrypt/mock_suite.h>
#include <vctool/commandline.h>
#include <vctool/command/root.h>
#include <vpr/allocator/malloc_allocator.h>

#include "../file/mock_file.h"

using namespace std;

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;
RCPR_IMPORT_slist;

/* start of the test suite. */
TEST_SUITE(commandline_opts_init);

/* If no arguments are provided, return an error. */
TEST(no_arguments)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
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

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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
            commandline_opts_init(
                &opts, alloc, &f, &suite, &builder_opts, 0, argv));

    /* clean up. */
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* If a -? is passed as an argument, invoke the help menu. */
TEST(question_mark_argument)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
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

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* the help command is set. */
    TEST_ASSERT(NULL != opts.cmd);

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* If a -h is passed as an argument, invoke the help menu. */
TEST(h_argument)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
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

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* the help command is set. */
    TEST_ASSERT(NULL != opts.cmd);

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* If a -N is passed as an argument, the non-interactive flag is set. */
TEST(N_argument)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string non_interactive_argument = "-N";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)non_interactive_argument.c_str(),
        (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* the help command is set. */
    TEST_ASSERT(NULL != opts.cmd);

    /* get the root command. */
    command* cmd = opts.cmd;
    while (cmd->next != NULL) cmd = cmd->next;
    root_command* root = (root_command*)cmd;

    /* the root command non-interactive flag is set. */
    TEST_EXPECT(root->non_interactive == true);

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* If a -k is passed as an argument, the key filename flag is set. */
TEST(k_argument)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string key_argument = "-k";
    string key_filename = "keyfile.cert";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)key_argument.c_str(),
        (char*)key_filename.c_str(), (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* the help command is set. */
    TEST_ASSERT(NULL != opts.cmd);

    /* get the root command. */
    command* cmd = opts.cmd;
    while (cmd->next != NULL) cmd = cmd->next;
    root_command* root = (root_command*)cmd;

    /* the root command key filename is set. */
    TEST_ASSERT(NULL != root->key_filename);
    TEST_EXPECT(key_filename == root->key_filename);

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* Passing the -k argument multiple times causes a failure. */
TEST(k_argument_multiple)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string key_argument = "-k";
    string key_filename = "keyfile.cert";
    string key_argument2 = "-k";
    string key_filename2 = "badkey.cert";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)key_argument.c_str(),
        (char*)key_filename.c_str(), (char*)key_argument2.c_str(),
        (char*)key_filename2.c_str(), (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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

    /* calling commandline_opts_init should fail. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS !=
            commandline_opts_init(
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* clean up. */
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* If a -o is passed as an argument, the output filename flag is set. */
TEST(o_argument)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string output_argument = "-o";
    string output_filename = "outfile.cert";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)output_argument.c_str(),
        (char*)output_filename.c_str(), (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* the help command is set. */
    TEST_ASSERT(NULL != opts.cmd);

    /* get the root command. */
    command* cmd = opts.cmd;
    while (cmd->next != NULL) cmd = cmd->next;
    root_command* root = (root_command*)cmd;

    /* the root command output filename is set. */
    TEST_ASSERT(NULL != root->output_filename);
    TEST_EXPECT(output_filename == root->output_filename);

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* Passing the -o argument multiple times causes a failure. */
TEST(o_argument_multiple)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string output_argument = "-o";
    string output_filename = "output.cert";
    string output_argument2 = "-o";
    string output_filename2 = "badoutput.cert";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)output_argument.c_str(),
        (char*)output_filename.c_str(), (char*)output_argument2.c_str(),
        (char*)output_filename2.c_str(), (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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

    /* calling commandline_opts_init should fail. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS !=
            commandline_opts_init(
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* clean up. */
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* If a -R is passed as an argument, the number of rounds can be changed. */
TEST(R_argument)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string rounds_argument = "-R";
    string rounds_number = "4999";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)rounds_argument.c_str(),
        (char*)rounds_number.c_str(), (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* the help command is set. */
    TEST_ASSERT(NULL != opts.cmd);

    /* get the root command. */
    command* cmd = opts.cmd;
    while (cmd->next != NULL) cmd = cmd->next;
    root_command* root = (root_command*)cmd;

    /* the root command rounds number is set. */
    TEST_EXPECT(4999 == root->key_derivation_rounds);

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* If a -R is passed as an argument, the number must be greater than zero. */
TEST(R_range_check)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string rounds_argument = "-R";
    string rounds_number = "-4999";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)rounds_argument.c_str(),
        (char*)rounds_number.c_str(), (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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

    /* calling commandline_opts_init should fail. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS !=
            commandline_opts_init(
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* By default, the dictionary in the root command structure is empty. */
TEST(empty_dict)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* the help command is set. */
    TEST_ASSERT(NULL != opts.cmd);

    /* get the root command. */
    command* cmd = opts.cmd;
    while (cmd->next != NULL) cmd = cmd->next;
    root_command* root = (root_command*)cmd;

    /* the root command dictionary is empty. */
    TEST_ASSERT(NULL != root->dict);
    TEST_EXPECT(0 == rbtree_count(root->dict));

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* a key-value pair is added to the dict with the -D option. */
TEST(D_argument)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string D_argument1 = "-Dfoo=bar";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)D_argument1.c_str(),
        (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;
    resource* val;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* the help command is set. */
    TEST_ASSERT(NULL != opts.cmd);

    /* get the root command. */
    command* cmd = opts.cmd;
    while (cmd->next != NULL) cmd = cmd->next;
    root_command* root = (root_command*)cmd;

    /* the root command dictionary has one entry. */
    TEST_ASSERT(NULL != root->dict);
    TEST_EXPECT(1 == rbtree_count(root->dict));

    /* We can retrieve this entry by name. */
    TEST_ASSERT(STATUS_SUCCESS == rbtree_find(&val, root->dict, "foo"));
    root_dict_kvp* foo_entry = (root_dict_kvp*)val;
    /* key is foo. */
    TEST_EXPECT(!strcmp("foo", foo_entry->key));
    /* value is bar. */
    TEST_EXPECT(!strcmp("bar", foo_entry->value));

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* Multiple key value pairs are processed. */
TEST(D_argument_multiple)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string D_argument1 = "-Dfoo=bar";
    string D_argument2 = "-Dabc=def";
    string D_argument3 = "-Dghi=jkl";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)D_argument1.c_str(),
        (char*)D_argument2.c_str(), (char*)D_argument3.c_str(),
        (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;
    resource* val;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* the help command is set. */
    TEST_ASSERT(NULL != opts.cmd);

    /* get the root command. */
    command* cmd = opts.cmd;
    while (cmd->next != NULL) cmd = cmd->next;
    root_command* root = (root_command*)cmd;

    /* the root command dictionary has three entries. */
    TEST_ASSERT(NULL != root->dict);
    TEST_EXPECT(3 == rbtree_count(root->dict));

    /* We can retrieve the "foo" entry by name. */
    TEST_ASSERT(STATUS_SUCCESS == rbtree_find(&val, root->dict, "foo"));
    root_dict_kvp* foo_entry = (root_dict_kvp*)val;
    /* key is foo. */
    TEST_EXPECT(!strcmp("foo", foo_entry->key));
    /* value is bar. */
    TEST_EXPECT(!strcmp("bar", foo_entry->value));

    /* We can retrieve the "abc" entry by name. */
    TEST_ASSERT(STATUS_SUCCESS == rbtree_find(&val, root->dict, "abc"));
    root_dict_kvp* abc_entry = (root_dict_kvp*)val;
    /* key is abc. */
    TEST_EXPECT(!strcmp("abc", abc_entry->key));
    /* value is def. */
    TEST_EXPECT(!strcmp("def", abc_entry->value));

    /* We can retrieve the "ghi" entry by name. */
    TEST_ASSERT(STATUS_SUCCESS == rbtree_find(&val, root->dict, "ghi"));
    root_dict_kvp* ghi_entry = (root_dict_kvp*)val;
    /* key is ghi. */
    TEST_EXPECT(!strcmp("ghi", ghi_entry->key));
    /* value is jkl. */
    TEST_EXPECT(!strcmp("jkl", ghi_entry->value));

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* A non-kvp def is not allowed. */
TEST(D_argument_bad)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string D_argument1 = "-Dfoo";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)D_argument1.c_str(),
        (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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

    /* calling commandline_opts_init should fail. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS !=
            commandline_opts_init(
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* A kvp wissing the value should fail. */
TEST(D_argument_missing_value)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string D_argument1 = "-Dfoo=";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)D_argument1.c_str(),
        (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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

    /* calling commandline_opts_init should fail. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS !=
            commandline_opts_init(
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* A kvp wissing the key should fail. */
TEST(D_argument_missing_key)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string D_argument1 = "-D=bar";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)D_argument1.c_str(),
        (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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

    /* calling commandline_opts_init should fail. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS !=
            commandline_opts_init(
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* If a -i is passed as an argument, the input filename flag is set. */
TEST(i_argument)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string input_argument = "-i";
    string input_filename = "infile.cert";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)input_argument.c_str(),
        (char*)input_filename.c_str(), (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* the help command is set. */
    TEST_ASSERT(NULL != opts.cmd);

    /* get the root command. */
    command* cmd = opts.cmd;
    while (cmd->next != NULL) cmd = cmd->next;
    root_command* root = (root_command*)cmd;

    /* the root command input filename is set. */
    TEST_ASSERT(NULL != root->input_filename);
    TEST_EXPECT(input_filename == root->input_filename);

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* Passing the -i argument multiple times causes a failure. */
TEST(i_argument_multiple)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string input_argument = "-i";
    string input_filename = "input.cert";
    string input_argument2 = "-i";
    string input_filename2 = "badinput.cert";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)input_argument.c_str(),
        (char*)input_filename.c_str(), (char*)input_argument2.c_str(),
        (char*)input_filename2.c_str(), (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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

    /* calling commandline_opts_init should fail. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS !=
            commandline_opts_init(
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* clean up. */
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* If a -E is passed as an argument, the endorse config filename flag is set. */
TEST(E_argument)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string endorse_argument = "-E";
    string endorse_filename = "endorse.cfg";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)endorse_argument.c_str(),
        (char*)endorse_filename.c_str(), (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* the help command is set. */
    TEST_ASSERT(NULL != opts.cmd);

    /* get the root command. */
    command* cmd = opts.cmd;
    while (cmd->next != NULL) cmd = cmd->next;
    root_command* root = (root_command*)cmd;

    /* the root command endorse config filename is set. */
    TEST_ASSERT(NULL != root->endorse_config_filename);
    TEST_EXPECT(endorse_filename == root->endorse_config_filename);

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* Passing the -E argument multiple times causes a failure. */
TEST(E_argument_multiple)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string endorse_argument = "-E";
    string endorse_filename = "endorse.cfg";
    string endorse_argument2 = "-E";
    string endorse_filename2 = "badendorse.cfg";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)endorse_argument.c_str(),
        (char*)endorse_filename.c_str(), (char*)endorse_argument2.c_str(),
        (char*)endorse_filename2.c_str(), (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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

    /* calling commandline_opts_init should fail. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS !=
            commandline_opts_init(
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* clean up. */
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* a permission is added to the permissions list with the -P option. */
TEST(P_argument)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string P_argument1 = "-Pfoo:bar";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)P_argument1.c_str(),
        (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;
    resource* val;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* the help command is set. */
    TEST_ASSERT(NULL != opts.cmd);

    /* get the root command. */
    command* cmd = opts.cmd;
    while (cmd->next != NULL) cmd = cmd->next;
    root_command* root = (root_command*)cmd;

    /* the root permissions list has one entry. */
    TEST_ASSERT(NULL != root->permissions);
    TEST_EXPECT(1 == slist_count(root->permissions));

    /* We can retrieve this entry. */
    slist_node* node = NULL;
    TEST_ASSERT(STATUS_SUCCESS == slist_head(&node, root->permissions));
    TEST_ASSERT(node != NULL);
    TEST_ASSERT(STATUS_SUCCESS == slist_node_child(&val, node));
    root_permission* foo_entry = (root_permission*)val;
    /* entity is foo. */
    TEST_EXPECT(!strcmp("foo", foo_entry->entity));
    /* moiety is bar. */
    TEST_EXPECT(!strcmp("bar", foo_entry->moiety));

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* Multiple permissions can be added. */
TEST(P_argument_multi)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string P_argument1 = "-Pfoo:bar";
    string P_argument2 = "-Pabc:def";
    string P_argument3 = "-Pghi:jkl";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)P_argument1.c_str(),
        (char*)P_argument2.c_str(), (char*)P_argument3.c_str(),
        (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;
    resource* val;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* the help command is set. */
    TEST_ASSERT(NULL != opts.cmd);

    /* get the root command. */
    command* cmd = opts.cmd;
    while (cmd->next != NULL) cmd = cmd->next;
    root_command* root = (root_command*)cmd;

    /* the root permissions list has three entries. */
    TEST_ASSERT(NULL != root->permissions);
    TEST_EXPECT(3 == slist_count(root->permissions));

    /* We can retrieve the first entry. */
    slist_node* node = NULL;
    TEST_ASSERT(STATUS_SUCCESS == slist_head(&node, root->permissions));
    TEST_ASSERT(node != NULL);
    TEST_ASSERT(STATUS_SUCCESS == slist_node_child(&val, node));
    root_permission* foo_entry = (root_permission*)val;
    /* entity is foo. */
    TEST_EXPECT(!strcmp("foo", foo_entry->entity));
    /* moiety is bar. */
    TEST_EXPECT(!strcmp("bar", foo_entry->moiety));

    /* We can retrieve the second entry. */
    slist_node* next = NULL;
    TEST_ASSERT(STATUS_SUCCESS == slist_node_next(&next, node));
    TEST_ASSERT(next != NULL);
    TEST_ASSERT(STATUS_SUCCESS == slist_node_child(&val, next));
    root_permission* abc_entry = (root_permission*)val;
    /* entity is foo. */
    TEST_EXPECT(!strcmp("abc", abc_entry->entity));
    /* moiety is bar. */
    TEST_EXPECT(!strcmp("def", abc_entry->moiety));

    /* We can retrieve the third entry. */
    node = next;
    TEST_ASSERT(STATUS_SUCCESS == slist_node_next(&next, node));
    TEST_ASSERT(next != NULL);
    TEST_ASSERT(STATUS_SUCCESS == slist_node_child(&val, next));
    root_permission* ghi_entry = (root_permission*)val;
    /* entity is foo. */
    TEST_EXPECT(!strcmp("ghi", ghi_entry->entity));
    /* moiety is bar. */
    TEST_EXPECT(!strcmp("jkl", ghi_entry->moiety));

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* invalid permissions cause a failure. */
TEST(P_argument_bad)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string P_argument1 = "-Pfoobar";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)P_argument1.c_str(),
        (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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

    /* calling commandline_opts_init should fail. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS !=
            commandline_opts_init(
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* a permission without an entity is invalid. */
TEST(P_argument_missing_entity)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string P_argument1 = "-P:bar";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)P_argument1.c_str(),
        (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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

    /* calling commandline_opts_init should fail. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS !=
            commandline_opts_init(
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* a permission without a moiety is invalid. */
TEST(P_argument_missing_moiety)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string P_argument1 = "-Pfoo:";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)P_argument1.c_str(),
        (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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

    /* calling commandline_opts_init should fail. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS !=
            commandline_opts_init(
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* a permission without an entity or moiety is invalid. */
TEST(P_argument_missing_entity_and_moiety)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string P_argument1 = "-P:";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)P_argument1.c_str(),
        (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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

    /* calling commandline_opts_init should fail. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS !=
            commandline_opts_init(
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* If a -v is NOT passed as an argument, then verbose mode is DISABLED. */
TEST(v_argument_negative)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* the help command is set. */
    TEST_ASSERT(NULL != opts.cmd);

    /* get the root command. */
    command* cmd = opts.cmd;
    while (cmd->next != NULL) cmd = cmd->next;
    root_command* root = (root_command*)cmd;

    /* the root command verbose flag is NOT set. */
    TEST_EXPECT(root->verbose == false);

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}

/* If a -v is passed as an argument, then verbose mode is enabled. */
TEST(v_argument)
{
    allocator_options_t alloc_opts;
    rcpr_allocator* alloc;
    vccrypt_suite_options_t suite;
    file f;
    vccert_builder_options_t builder_opts;
    string exe_name = "vctool";
    string verbose_argument = "-v";
    string help_argument = "help";
    char* argv[] = {
        (char*)exe_name.c_str(), (char*)verbose_argument.c_str(),
        (char*)help_argument.c_str() };
    int argc = sizeof(argv) / sizeof(char*);
    commandline_opts opts;

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create RCPR allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

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
                &opts, alloc, &f, &suite, &builder_opts, argc, argv));

    /* the help command is set. */
    TEST_ASSERT(NULL != opts.cmd);

    /* get the root command. */
    command* cmd = opts.cmd;
    while (cmd->next != NULL) cmd = cmd->next;
    root_command* root = (root_command*)cmd;

    /* the root command verbose flag is set. */
    TEST_EXPECT(root->verbose == true);

    /* clean up. */
    dispose((disposable_t*)&opts);
    dispose((disposable_t*)&builder_opts);
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
    dispose((disposable_t*)&alloc_opts);
}
