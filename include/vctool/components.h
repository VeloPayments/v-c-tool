/**
 * \file include/vctool/components.h
 *
 * \brief Components enumeration.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef VCTOOL_COMPONENTS_HEADER_GUARD
#define VCTOOL_COMPONENTS_HEADER_GUARD

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief vctool Components.
 */
enum vctool_components
{
    /**
     * \brief General Components.
     */
    VCTOOL_COMPONENT_GENERAL = 0x00U,

    /**
     * \brief file Component.
     */
    VCTOOL_COMPONENT_FILE = 0x01U,

    /**
     * \brief commandline Component.
     */
    VCTOOL_COMPONENT_COMMANDLINE = 0x02U,

    /**
     * \brief readpassword Component.
     */
    VCTOOL_COMPONENT_READPASSWORD = 0x03U,

    /**
     * \brief certificate Component.
     */
    VCTOOL_COMPONENT_CERTIFICATE = 0x04U,

    /**
     * \brief backup Component.
     */
    VCTOOL_COMPONENT_BACKUP = 0x05U,

    /**
     * \brief endorse Component.
     */
    VCTOOL_COMPONENT_ENDORSE = 0x06U,

    /**
     * \brief keygen Component.
     */
    VCTOOL_COMPONENT_KEYGEN = 0x07U,

    /**
     * \brief pubkey Component.
     */
    VCTOOL_COMPONENT_PUBKEY = 0x08U,
};

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*VCTOOL_COMPONENTS_HEADER_GUARD*/
