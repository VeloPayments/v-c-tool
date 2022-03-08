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
#include <vctool/command/root.h>
#include <vpr/allocator/malloc_allocator.h>

#include "../file/mock_file.h"

using namespace std;

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

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
