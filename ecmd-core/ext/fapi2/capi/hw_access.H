/* IBM_PROLOG_BEGIN_TAG                                                   */
/* 
 * Copyright 2017 IBM International Business Machines Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* IBM_PROLOG_END_TAG                                                     */

/// @file hw_access.H
///
/// @brief Hardware access functions that needs to be specialized for
/// platform implementation.
///

#ifndef _FAPI2_HWACCESS_H_
#define _FAPI2_HWACCESS_H_

// variable_buffer isn't supported on PPE
#ifndef __PPE__
    #include <variable_buffer.H>
#endif

#include <plat_hw_access.H>
#include <fapi2_hw_access.H>
#include <plat_utils.H>

namespace fapi2
{
//--------------------------------------------------------------------------
// PIB Error Functions
//--------------------------------------------------------------------------

/// @brief Sets the PIB error mask - platform dependant
/// @param[in]  i_mask     The new error mask
// note: this can be moved  to a C file if desired
inline void setPIBErrorMask(uint8_t i_mask)
{
    // Keeps the compiler from complaining about the unused i_mask
    static_cast<void>(i_mask);

    return;
}

/// @brief Gets the PIB error mask - platform dependant
/// @return uint8_t The current PIB error mask
// note: this can be moved  to a C file if desired
inline uint8_t getPIBErrorMask(void)
{
    return 0;
}

//--------------------------------------------------------------------------
// Operational Mode Error Functions
//--------------------------------------------------------------------------

/// @brief Sets the operational mode
/// @param[in]  i_mode     The new mode
// note: this can be moved  to a C file if desired
inline void setOpMode(const OpModes i_mode)
{
    // Keeps the compiler from complaining about the unused i_mode
    static_cast<void>(i_mode);

    // No-op for now. Should set thread-local operational mode
    return;
}

/// @brief Gets the operational mode
/// @return the operational mode
// note: this can be moved  to a C file if desired
inline OpModes getOpMode(void)
{
    // No-op for now. Should read thread-local operational mode
    return NORMAL;
}

//--------------------------------------------------------------------------
// Multicast mapping functions
//--------------------------------------------------------------------------

/// @brief Set up or replace the multicast group mapping for a given chip
/// @param[in] i_chip     The chip whose multicast map is to be replaced
/// @param[in] i_mappings A list of multicast group mappings
/// @return FAPI2_RC_SUCCESS if the map was updated, otherwise error code.
///
/// This replaces the given chip's map of abstract multicast groups to chip
/// specific hardware values with a new map. Any abstract group not explicitly
/// mapped via i_mappings will be unmapped after this call returns success,
/// and attempting to create multicast targets targeting unmapped groups will
/// result in error.
/// i_mappings may contain an arbitrary amount of abstract/HW pairs, but the
/// function may return an error if too many mappings are specified.
template< MulticastType M, typename V >
inline ReturnCode setMulticastGroupMap(const Target<TARGET_TYPE_PROC_CHIP, M, V>& i_chip,
                                       const std::vector< MulticastGroupMapping >& i_mappings)
{
    ReturnCode l_rc;
    bool l_traceit = fapi2plat::isScanTraceEnabled();

    // call the platform implementation
    l_rc = fapi2plat::setMulticastGroupMap( i_chip, i_mappings );

    if (l_rc != FAPI2_RC_SUCCESS)
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_chip, target_string, MAX_ECMD_STRING_LEN);
        FAPI_ERR("setMulticastGroupMap failed - Target %s", target_string);
    }

    if( l_traceit )
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_chip, target_string, MAX_ECMD_STRING_LEN);
        FAPI_SCAN( "TRACE : SETMCGRPMAP : %s : ", target_string );
    }

    return l_rc;
}

//------------------------------------------------------------------------------
// HW Communication Functions to be implemented at the platform layer.
//------------------------------------------------------------------------------

///
/// @brief Platform-level implementation of getScom()
/// @tparam K template parameter, passed in target.
/// @param[in]  i_target   HW target to operate on.
/// @param[in]  i_address  SCOM register address to read from.
/// @param[out] o_data     Buffer that holds data read from HW target.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
template< TargetType K, MulticastType M, typename V >
inline ReturnCode getScom(const Target<K, M, V>& i_target,
                          const uint64_t i_address,
                          buffer<uint64_t>& o_data)
{
    ReturnCode l_rc;
    bool l_traceit = fapi2plat::isScanTraceEnabled();

    // call the platform implementation
    l_rc = fapi2plat::getScom( i_target, i_address, o_data );

    if (l_rc != FAPI2_RC_SUCCESS)
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_ERR("getScom failed - Target %s, Addr " UINT64_HEX16_PRESC_FORMAT,
                 target_string, i_address);
    }

    if( l_traceit )
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_SCAN( "TRACE : GETSCOM     : %s : " UINT64_HEX16_PRESC_FORMAT " " UINT64_HEX16_PRESC_FORMAT ,
                   target_string,
                   i_address,
                   static_cast<uint64_t>(o_data) );
    }

    return l_rc;
}

/// @brief Platform-level implementation of putScom()
/// @tparam K template parameter, passed in target.
/// @param[in] i_target     HW target to operate on.
/// @param[in] i_address    SCOM register address to write to.
/// @param[in] i_data       Buffer that holds data to write into address.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
template< TargetType K, MulticastType M, typename V >
inline ReturnCode putScom(const Target<K, M, V>& i_target,
                          const uint64_t i_address,
                          const buffer<uint64_t> i_data)
{
    ReturnCode l_rc;
    bool l_traceit = fapi2plat::isScanTraceEnabled();

    // call the platform implementation
    l_rc = fapi2plat::putScom( i_target, i_address, i_data );

    if (l_rc != FAPI2_RC_SUCCESS)
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_ERR("putScom failed - Target %s, Addr " UINT64_HEX16_PRESC_FORMAT,
                 target_string, i_address);
    }

    if( l_traceit )
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_SCAN( "TRACE : PUTSCOM     : %s : " UINT64_HEX16_PRESC_FORMAT " " UINT64_HEX16_PRESC_FORMAT ,
                   target_string,
                   i_address,
                   static_cast<uint64_t>(i_data) );
    }

    return l_rc;
}

/// @brief Platform-level implementation of putScomUnderMask()
/// @tparam K template parameter, passed in target.
/// @param[in] i_target     HW target to operate on.
/// @param[in] i_address    SCOM register address to write to.
/// @param[in] i_data       Buffer that holds data to write into address.
/// @param[in] i_mask       Buffer that holds the mask value.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
template< TargetType K, MulticastType M, typename V >
inline ReturnCode putScomUnderMask(const Target<K, M, V>& i_target,
                                   const uint64_t i_address,
                                   const buffer<uint64_t> i_data,
                                   const buffer<uint64_t> i_mask)
{
    ReturnCode l_rc;
    bool l_traceit = fapi2plat::isScanTraceEnabled();

    // call the platform implementation
    l_rc = fapi2plat::putScomUnderMask( i_target, i_address, i_data, i_mask );

    if (l_rc != FAPI2_RC_SUCCESS)
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_ERR("putScomUnderMask failed - Target %s, Addr " UINT64_HEX16_PRESC_FORMAT,
                 target_string, i_address);
    }

    if( l_traceit )
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_SCAN( "TRACE : PUTSCOMMASK : %s : " UINT64_HEX16_PRESC_FORMAT " " UINT64_HEX16_PRESC_FORMAT " "
                   UINT64_HEX16_PRESC_FORMAT,
                   target_string,
                   i_address,
                   static_cast<uint64_t>(i_data),
                   static_cast<uint64_t>(i_mask) );
    }

    return l_rc;
}

///
/// @brief Platform-level implementation called by getCfamRegister()
/// Hardware procedures writers will not call this function.
/// @tparam K template parameter, passed in target.
/// @param[in]  i_target   HW target to operate on.
/// @param[in]  i_address  CFAM address to read from.
/// @param[out] o_data     32-bit buffer that holds data read from HW target.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
template< TargetType K, MulticastType M, typename V >
inline ReturnCode getCfamRegister(const Target<K, M, V>& i_target,
                                  const uint32_t i_address,
                                  buffer<uint32_t>& o_data)
{
    ReturnCode l_rc;
    bool l_traceit = fapi2plat::isScanTraceEnabled();

    // call the platform implementation
    l_rc = fapi2plat::getCfamRegister( i_target, i_address, o_data );

    if (l_rc != FAPI2_RC_SUCCESS)
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_ERR("getCfamRegister failed - Target %s, Addr %.8X",
                 target_string, i_address);
    }

    if( l_traceit )
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_SCAN( "TRACE : GETCFAMREG  : %s : %.8X %.8X",
                   target_string,
                   i_address,
                   static_cast<uint32_t>(o_data) );
    }

    return l_rc;
}

///
/// @brief Platform-level implementation of putCfamRegister()
/// Hardware procedures writers will not call this function.
/// @tparam K template parameter, passed in target.
/// @param[in]  i_target   HW target to operate on.
/// @param[in]  i_address  CFAM address to write to.
/// @param[out] i_data     32-bit buffer that holds data to write into address.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
template< TargetType K, MulticastType M, typename V >
inline ReturnCode putCfamRegister(const Target<K, M, V>& i_target,
                                  const uint32_t i_address,
                                  const buffer<uint32_t> i_data)
{
    ReturnCode l_rc;
    bool l_traceit = fapi2plat::isScanTraceEnabled();

    // call the platform implementation
    l_rc = fapi2plat::putCfamRegister( i_target, i_address, i_data );

    if (l_rc != FAPI2_RC_SUCCESS)
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_ERR("putCfamRegister failed - Target %s, Addr %.8X",
                 target_string, i_address);
    }

    if( l_traceit )
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_SCAN( "TRACE : PUTCFAMREG  : %s : %.8X %.8X",
                   target_string,
                   i_address,
                   static_cast<uint32_t>(i_data) );
    }

    return l_rc;
}


///
/// @brief Platform-level implementation of modifyCfamRegister()
/// Hardware procedures writers will not call this function.
/// @tparam K template parameter, passed in target.
/// @param[in]  i_target     HW target to operate on.
/// @param[in]  i_address    CFAM register address to modify.
/// @param[out] i_data       32-bit buffer that holds data to modify.
/// @param[in]  i_modifyMode The modify mode (or/and/xor).
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
template< TargetType K, MulticastType M, typename V >
inline ReturnCode modifyCfamRegister(const Target<K, M, V>& i_target,
                                     const uint32_t i_address,
                                     const buffer<uint32_t> i_data,
                                     const fapi2::ChipOpModifyMode i_modifyMode)
{
#if 0
    std::cout << std::hex << "   modifyCfamRegister "
              << "target: {" << i_target.getType() << ","
              << uint64_t(i_target) << "}; "
              << "address: " << i_address << "; "
              << "input modifying data: " << uint32_t(i_data) << "; "
              << "input ChipOpModifyMode: " << i_modifyMode
              << std::dec << std::endl;
#endif
    return FAPI2_RC_SUCCESS;
}

/// @brief Platform-level implementation  of putRing()
/// Hardware procedures writers will not call this function.
/// @tparam K template parameter, passed in target.
/// @param[in] i_target   Target to operate on.
/// @param[in] i_ringID   Ring ID that will identify the Ring in the image.
/// @param[in] i_ringMode Ring operation mode.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
template< TargetType K, MulticastType M, typename V >
ReturnCode putRing(const Target<K, M, V>& i_target,
                   const RingId_t i_ringID,
                   const RingMode i_ringMode)
{
    fapi2::ReturnCode l_rc;
    bool l_traceit = fapi2plat::isScanTraceEnabled();

    // call the platform implementation
    l_rc = fapi2plat::putRing( i_target, i_ringID, i_ringMode );

    if (l_rc != FAPI2_RC_SUCCESS)
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_ERR("putRing failed - Target %s, RingID %d",
                 target_string, i_ringID);
    }

    if( l_traceit )
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_SCAN( "TRACE : PUTRING     : %s : %d %d",
                   target_string,
                   i_ringID,
                   i_ringMode );
    }

    return l_rc;
}

/// @brief Platform-level implementation  of putRing()
/// Hardware procedures writers will not call this function.
/// @tparam K template parameter, passed in target.
/// @param[in] i_target   Target to operate on.
/// @param[in] i_ringID   Ring ID name that will identify the Ring in the image.
/// @param[in] i_ringMode Ring operation mode.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
template< TargetType K, MulticastType M, typename V >
ReturnCode putRing(const Target<K, M, V>& i_target,
                   const char* i_ringID,
                   const RingMode i_ringMode)
{
    fapi2::ReturnCode l_rc;
    bool l_traceit = fapi2plat::isScanTraceEnabled();

    // call the platform implementation
    l_rc = fapi2plat::putRing( i_target, i_ringID, i_ringMode );

    if (l_rc != FAPI2_RC_SUCCESS)
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_ERR("putRing failed - Target %s, RingID %s",
                 target_string, i_ringID);
    }

    if( l_traceit )
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_SCAN( "TRACE : PUTRING     : %s : %s %d",
                   target_string,
                   i_ringID,
                   i_ringMode );
    }

    return l_rc;
}

// variable_buffer isn't supported on PPE
#ifndef __PPE__
///
/// @brief Platform-level implementation of getRing()
/// Hardware procedures writers will not call this function.
/// @tparam K template parameter, passed in target.
/// @param[in]  i_target   HW target to operate on.
/// @param[in]  i_address  Ring address to read from.
/// @param[out] o_data     Buffer that holds ring data read from HW target.
/// @param[in]  i_ringMode Ring operation mode.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
template< TargetType K, MulticastType M, typename V >
inline ReturnCode getRing(const Target<K, M, V>& i_target,
                          const scanRingId_t i_address,
                          variable_buffer& o_data,
                          const RingMode i_ringMode)
{
    fapi2::ReturnCode l_rc;
    bool l_traceit = fapi2plat::isScanTraceEnabled();

    // call the platform implementation
    l_rc = fapi2plat::getRing( i_target, i_address, o_data, i_ringMode );

    if (l_rc != FAPI2_RC_SUCCESS)
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_ERR("getRing failed - Target %s, Addr 0x" UINT64_HEX16_PRESC_FORMAT,
                 target_string, i_address);
    }

    if( l_traceit )
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        uint64_t l_data = 0;
        o_data.extract(l_data, 0, 64);
        FAPI_SCAN( "TRACE : GETRING     : %s : " UINT64_HEX16_PRESC_FORMAT " " UINT64_HEX16_PRESC_FORMAT,
                   target_string,
                   i_address,
                   l_data );
    }

    return l_rc;
}

#if 0
/// @brief Platform-level implementation  of putRing()
/// Hardware procedures writers will not call this function.
/// @tparam K template parameter, passed in target.
/// @param[in] i_target   Target to operate on.
/// @param[in] i_address  Ring address to write to.
/// @param[in] i_data     Buffer that contains RS4 compressed ring data
///                       to write into address
/// @param[in] i_ringMode Ring operation mode.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
template< TargetType K, MulticastType M, typename V >
inline ReturnCode putRing(const Target<K, M, V>& i_target,
                          const scanRingId_t i_address,
                          const variable_buffer& i_data,
                          const RingMode i_ringMode)
{
    fapi2::ReturnCode l_rc;
    bool l_traceit = fapi2plat::isScanTraceEnabled();

    // call the platform implementation
    l_rc = fapi2plat::putRing( i_target, i_address, i_data, i_ringMode );

    if (l_rc != FAPI2_RC_SUCCESS)
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_ERR("putRing failed - Target %s, Addr 0x" UINT64_HEX16_PRESC_FORMAT,
                 target_string, i_address);
    }

    if( l_traceit )
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        uint64_t l_data = 0;
        i_data.extract(l_data, 0, 64);
        FAPI_SCAN( "TRACE : PUTRING     : %s : " UINT64_HEX16_PRESC_FORMAT " " UINT64_HEX16_PRESC_FORMAT,
                   target_string,
                   i_address,
                   l_data );
    }

    return l_rc;
}
#endif

/// @brief Platform-level implementation of modifyRing()
/// @tparam K template parameter, passed in target.
/// @param[in] i_target     Target to operate on.
/// @param[in] i_address    Ring address to modify.
/// @param[in] i_data       Buffer that contains RS4 compressed ring data
///                         to be modified.
/// @param[in] i_modifyMode The modify mode (or/and/xor)
/// @param[in] i_ringMode   Ring operation mode.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
template< TargetType K, MulticastType M, typename V >
inline ReturnCode modifyRing(const Target<K, M, V>& i_target,
                             const scanRingId_t i_address,
                             const variable_buffer& i_data,
                             const ChipOpModifyMode i_modifyMode,
                             const RingMode i_ringMode)
{
#if 0
    std::cout << std::hex << "   modifyRing "
              << "target: {" << i_target.getType() << ","
              << uint64_t(i_target) << "}; "
              << "address: " << i_address << "; "
              << "input ChipOpModifyMode: " << i_modifyMode << "; "
              << "ring mode: " << i_ringMode << "; "
              << "first element of the input data: " << i_data()[0]
              << std::endl;
#endif
    return FAPI2_RC_SUCCESS;
}
#endif

// --------------------------------------------------------------------------
// NOTE:
// Implement platform Spy access functions if platform supports them.
// --------------------------------------------------------------------------

// variable_buffer isn't supported on PPE
#ifndef __PPE__
/// @brief Reads a spy from a chip.
/// @tparam K template parameter, passed in target.
/// @param[in]  i_target   Target to operate on.
/// @param[in]  i_spyId    Id of the spy whose data to be read.
/// @param[out] o_data     Buffer that holds data read from HW target.
/// @return fapi2::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
/// @note:  The string version is only supported for cronus.
///
///         The fapi design to support both FSP and cronus use of get and
///         put spy functions is dependant on the SPY names being expanded
///         to resemble a valid C identifier. This design places some
///         restrictions on the SPY names which can be used.
///
///         1. if the spy name contains a # procedure writers should replace
///         it with an __P__ for example -
///
///         ABUS.RX0.RXPACKS#0.RXPACK.RD.LC.LC.ACT_DIS
///         becomes
///         ABUS.RX0.RXPACKS__P__0.RXPACK.RD.LC.LC.ACT_DIS
///
///         2. if the spy name has a number following a "." it must have an
///         underscore prepended to the number.
///
///         EH.TPCHIP.2KX100_ARY_CLK_EDGES_DLY
///         becomes
///         EH.TPCHIP._2KX100_ARY_CLK_EDGES_DLY
///
///         Example SPY name:
///         The hardware procedure should call the function like:
///
///          ABUS.RX0.RXPACKS#0.RXPACK.RD.LC.LC.ACT_DIS
///
///          fapi2::ReturnCode rc = fapiGetSpy( targ,
///                  ABUS.RX0.RXPACKS__P__0.RXPACK.RD.LC.LC.ACT_DIS, data );
///
/// @note The ID is not in quotes the fapi code will handle adding
///       the quotes for the cronus environment
///
#ifdef FAPI_SUPPORT_SPY_AS_ENUM
template< TargetType K, MulticastType M, typename V >
inline ReturnCode getSpy(const Target<K, M, V>& i_target,
                         const spyId_t i_spyId,
                         variable_buffer& o_data)
{
    static_assert(K == 0, "implement getSpy (enum)");
    return ~FAPI2_RC_SUCCESS;
}
#endif
#ifdef FAPI_SUPPORT_SPY_AS_STRING
template< TargetType K, MulticastType M, typename V >
inline ReturnCode getSpy(const Target<K, M, V>& i_target,
                         const char* const i_spyId,
                         variable_buffer& o_data)
{
    fapi2::ReturnCode l_rc;
    bool l_traceit = fapi2plat::isScanTraceEnabled();

    // call the platform implementation
    l_rc = fapi2plat::getSpy( i_target, i_spyId, o_data );

    if (l_rc != FAPI2_RC_SUCCESS)
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_ERR("getSpy failed - Target %s, SpyId %s",
                 target_string, i_spyId);
    }

    if( l_traceit )
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        uint64_t l_data = 0;
        o_data.extract(l_data, 0, 64);
        FAPI_SCAN( "TRACE : GETSPY      : %s : %s " UINT64_HEX16_PRESC_FORMAT,
                   target_string,
                   i_spyId,
                   l_data );
    }

    return l_rc;
}
#endif
/// @brief Writes a spy on a chip.
/// @tparam K template parameter, passed in target.
/// @param[in]  i_target   Target to operate on.
/// @param[in]  i_spyId    Id of the spy to write data to.
/// @param[out] i_data     Buffer that holds data to write into spy.
/// @return fapi2::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
/// @note:  The string version is only supported for cronus.
///
///         The fapi design to support both FSP and cronus use of get and
///         put spy functions is dependent on the SPY names being expanded
///         to resemble a valid C identifier. This design places some
///         restrictions on the SPY names which can be used.
///
///         1. if the spy name contains a # procedure writers should replace
///         is with an __P__ for example -
///
///         ABUS.RX0.RXPACKS#0.RXPACK.RD.LC.LC.ACT_DIS
///         becomes
///         ABUS.RX0.RXPACKS__P__0.RXPACK.RD.LC.LC.ACT_DIS
///
///         2. if the spy name has a number following a "." it must have an
///         underscore prepended to the number.
///
///         EH.TPCHIP.2KX100_ARY_CLK_EDGES_DLY
///         becomes
///         EH.TPCHIP._2KX100_ARY_CLK_EDGES_DLY
///
///         Example SPY name:
///         The hardware procedure should call the function like:
///
///          ABUS.RX0.RXPACKS#0.RXPACK.RD.LC.LC.ACT_DIS
///
///          fapi2::ReturnCode rc = fapiPutSpy( targ,
///                  ABUS.RX0.RXPACKS__P__0.RXPACK.RD.LC.LC.ACT_DIS, data );
///
/// @note The ID is not in quotes the fapi code will handle adding
///         the quotes for the cronus environment
///
#ifdef FAPI_SUPPORT_SPY_AS_ENUM
template< TargetType K, MulticastType M, typename V >
inline ReturnCode putSpy(const Target<K, M, V>& i_target,
                         const spyId_t i_spyId,
                         const variable_buffer& i_data)
{
    static_assert(K == 0, "implement putSpy (enum)");
    return ~FAPI2_RC_SUCCESS;
}
#endif
#ifdef FAPI_SUPPORT_SPY_AS_STRING
template< TargetType K, MulticastType M, typename V >
inline ReturnCode putSpy(const Target<K, M, V>& i_target,
                         const char* const i_spyId,
                         const variable_buffer& i_data)
{
    fapi2::ReturnCode l_rc;
    bool l_traceit = fapi2plat::isScanTraceEnabled();

    // call the platform implementation
    l_rc = fapi2plat::putSpy( i_target, i_spyId, i_data );

    if (l_rc != FAPI2_RC_SUCCESS)
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_ERR("putSpy failed - Target %s, SpyId %s",
                 target_string, i_spyId);
    }

    if( l_traceit )
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        uint64_t l_data = 0;
        i_data.extract(l_data, 0, 64);
        FAPI_SCAN( "TRACE : PUTSPY      : %s : %s " UINT64_HEX16_PRESC_FORMAT,
                   target_string,
                   i_spyId,
                   l_data );
    }

    return l_rc;
}

template<TargetType K, MulticastType M, typename V >
inline ReturnCode putSpyWithCare(const Target<K, M, V>& i_target,
                                 const char* const i_spyId,
                                 const variable_buffer& i_data,
                                 const variable_buffer& i_care)
{
    // ignore i_care
    return putSpy(i_target, i_spyId, i_data);
}

#endif
/// @brief Writes spy data into a buffer holding ring data image
///        This API is used by L2/L3 repair to put column repair data
///        into a ring buffer image.
/// @tparam K template parameter, passed in target.
/// @param[in]  i_target      Target to operate on.
/// @param[in]  i_spyId       Id of the spy.
/// @param[in]  i_data        Buffer that holds spy data to write into ring
///                           image.
/// @param[out] o_imageData   Buffer that holds updated ring image.
/// @return fapi2::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
/// @note:  The string version is only supported for cronus.
///
///         The fapi design to support both FSP and cronus use of get and
///         put spy functions is dependent on the SPY names being expanded
///         to resemble a valid C identifier. This design places some
///         restrictions on the SPY names which can be used.
///
///         See fapiPutSpy for details on spy id specifics.
///
#ifdef FAPI_SUPPORT_SPY_AS_ENUM
template< TargetType K, MulticastType M, typename V >
inline ReturnCode putSpyImage(const Target<K, M, V>& i_target,
                              const spyId_t i_spyId,
                              const variable_buffer& i_data,
                              variable_buffer& o_imageData)
{
    static_assert(K == 0, "implement putSpyImage (enum)");
    return ~FAPI2_RC_SUCCESS;
}
#endif
#ifdef FAPI_SUPPORT_SPY_AS_STRING
template< TargetType K, MulticastType M, typename V >
inline ReturnCode putSpyImage(const Target<K, M, V>& i_target,
                              const char* const i_spyId,
                              const variable_buffer& i_data,
                              variable_buffer& o_imageData)
{
    fapi2::ReturnCode l_rc;
    bool l_traceit = fapi2plat::isScanTraceEnabled();

    // call the platform implementation
    l_rc = fapi2plat::putSpyImage( i_target, i_spyId, i_data, o_imageData );

    if (l_rc != FAPI2_RC_SUCCESS)
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_ERR("putSpyImage failed - Target %s, SpyId %s",
                 target_string, i_spyId);
    }

    if( l_traceit )
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        uint64_t l_data = 0;
        i_data.extract(l_data, 0, 64);
        FAPI_SCAN( "TRACE : PUTSPYIMAGE : %s : %s " UINT64_HEX16_PRESC_FORMAT,
                   target_string,
                   i_spyId,
                   l_data );
    }

    return l_rc;
}
#endif
/// @brief Reads spy data from a ring image buffer
/// @param[in]   i_target      Target to operate on
/// @param[in]   i_spyId       The spy's id
/// @param[out]  o_data        Buffer that holds data read from ring image.
/// @param[in]   i_imageData   Buffer that holds ring image to read data
///                            from.
/// @return fapi2::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
/// @note:  The string version is only supported for cronus.
///
///         The fapi design to support both FSP and cronus use of get and
///         put spy functions is dependent on the SPY names being expanded
///         to resemble a valid C identifier. This design places some
///         restrictions on the SPY names which can be used.
///
///         See fapiPutSpy for details on spy id specifics.
///
#ifdef FAPI_SUPPORT_SPY_AS_ENUM
template< TargetType K, MulticastType M, typename V >
inline ReturnCode getSpyImage(const Target<K, M, V>& i_target,
                              const spyId_t i_spyId,
                              variable_buffer& o_data,
                              const variable_buffer& i_imageData)
{
    static_assert(K == 0, "implement getSpyImage (enum)");
    return ~FAPI2_RC_SUCCESS;
}
#endif
#ifdef FAPI_SUPPORT_SPY_AS_STRING
template< TargetType K, MulticastType M, typename V >
inline ReturnCode getSpyImage(const Target<K, M, V>& i_target,
                              const char* const i_spyId,
                              variable_buffer& o_data,
                              const variable_buffer& i_imageData)
{
    fapi2::ReturnCode l_rc;
    bool l_traceit = fapi2plat::isScanTraceEnabled();

    // call the platform implementation
    l_rc = fapi2plat::getSpyImage( i_target, i_spyId, o_data, i_imageData );

    if (l_rc != FAPI2_RC_SUCCESS)
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        FAPI_ERR("getSpyImage failed - Target %s, SpyId %s",
                 target_string, i_spyId);
    }

    if( l_traceit )
    {
        char target_string[MAX_ECMD_STRING_LEN];
        fapi2::toString(i_target, target_string, MAX_ECMD_STRING_LEN);
        uint64_t l_data = 0;
        o_data.extract(l_data, 0, 64);
        FAPI_SCAN( "TRACE : GETSPY      : %s : %s " UINT64_HEX16_PRESC_FORMAT,
                   target_string,
                   i_spyId,
                   l_data );
    }

    return l_rc;
}
#endif

#endif // PPE

};

#endif // _FAPI2_HWACCESS_H_
