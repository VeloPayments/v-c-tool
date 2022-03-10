/**
 * \file include/vctool/command/root.h
 *
 * \brief Root command structure.
 *
 * \copyright 2020-2022 Velo Payments.  See License.txt for license terms.
 */

#ifndef  VCTOOL_COMMAND_ROOT_HEADER_GUARD
# define VCTOOL_COMMAND_ROOT_HEADER_GUARD

#include <rcpr/allocator.h>
#include <rcpr/rbtree.h>
#include <rcpr/resource/protected.h>
#include <stdbool.h>
#include <stdio.h>
#include <vctool/commandline.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/* the default number of rounds to use for deriving a key. */
#define ROOT_COMMAND_DEFAULT_KEY_DERIVATION_ROUNDS      50000

typedef struct root_command
{
    command hdr;
    RCPR_SYM(allocator)* alloc;
    bool help_requested;
    bool non_interactive;
    char* input_filename;
    char* output_filename;
    char* endorse_config_filename;
    char* key_filename;
    unsigned int key_derivation_rounds;
    RCPR_SYM(rbtree)* dict;
} root_command;

typedef struct root_dict_kvp
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    char* key;
    char* value;
} root_dict_kvp;

/**
 * \brief Initialize a root command structure.
 *
 * \param root          The root command structure to initialize.
 * \param alloc         The RCPR allocator to use.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int root_command_init(root_command* root, RCPR_SYM(allocator)* alloc);

/**
 * \brief Dispatch root level commands.
 *
 * \param opts          The command-line option structure.
 * \param argc          The argument count.
 * \param argv          The argument vector.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int dispatch_root_commands(commandline_opts* opts, int argc, char* argv[]);

/**
 * \brief Add a key-value pair in the form of a string of "key=value" to the
 * root dictionary.
 *
 * \param root          The command-line root command.
 * \param kvp           The key-value pair to add.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int root_dict_add(root_command* root, const char* kvp);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_COMMAND_ROOT_HEADER_GUARD*/
